/* Meta support for 32-bit ELF
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

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
#include "elf32-metag.h"
#include "elf/metag.h"

#define GOT_ENTRY_SIZE 4
#define ELF_DYNAMIC_INTERPRETER "/lib/ld-uClibc.so.0"

/* ABI version:
    0 - original
    1 - with GOT offset */
#define METAG_ELF_ABI_VERSION 1

static const unsigned int plt0_entry[] =
  {
    0x02000005, /* MOVT D0Re0, #HI(GOT+4) */
    0x02000000, /* ADD  D0Re0, D0Re0, #LO(GOT+4) */
    0xb70001e3, /* SETL [A0StP++], D0Re0, D1Re0 */
    0xc600012a, /* GETD PC, [D0Re0+#4] */
    0xa0fffffe  /* NOP */
  };

static const unsigned int plt0_pic_entry[] =
  {
    0x82900001, /* ADDT A0.2, CPC0, #0 */
    0x82100000, /* ADD  A0.2, A0.2, #0 */
    0xa3100c20, /* MOV  D0Re0, A0.2 */
    0xb70001e3, /* SETL [A0StP++], D0Re0, D1Re0 */
    0xc600012a, /* GETD PC, [D0Re0+#4] */
  };

static const unsigned int plt_entry[] =
  {
    0x82100005, /* MOVT A0.2, #HI(GOT+off) */
    0x82100000, /* ADD  A0.2, A0.2, #LO(GOT+off) */
    0xc600806a, /* GETD PC, [A0.2] */
    0x03000004, /* MOV  D1Re0, #LO(offset) */
    0xa0000000  /* B    PLT0 */
  };

static const unsigned int plt_pic_entry[] =
  {
    0x82900001, /* ADDT A0.2, CPC0, #HI(GOT+off) */
    0x82100000, /* ADD  A0.2, A0.2, #LO(GOT+off) */
    0xc600806a, /* GETD PC, [A0.2] */
    0x03000004, /* MOV  D1Re0, #LO(offset) */
    0xa0000000  /* B    PLT0 */
  };

/* Variable names follow a coding style.
   Please follow this (Apps Hungarian) style:

   Structure/Variable		   Prefix
   elf_link_hash_table		   "etab"
   elf_link_hash_entry		   "eh"

   elf_metag_link_hash_table	   "htab"
   elf_metag_link_hash_entry	   "hh"

   bfd_link_hash_table		   "btab"
   bfd_link_hash_entry		   "bh"

   bfd_hash_table containing stubs "bstab"
   elf_metag_stub_hash_entry	   "hsh"

   Always remember to use GNU Coding Style.  */

#define PLT_ENTRY_SIZE sizeof(plt_entry)

static reloc_howto_type elf_metag_howto_table[] =
{
  /* High order 16 bit absolute.  */
  HOWTO (R_METAG_HIADDR16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_HIADDR16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low order 16 bit absolute.  */
  HOWTO (R_METAG_LOADDR16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_LOADDR16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit absolute.  */
  HOWTO (R_METAG_ADDR32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_ADDR32",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* No relocation.  */
  HOWTO (R_METAG_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 19 bit pc relative */
  HOWTO (R_METAG_RELBRANCH,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 19,			/* bitsize */
	 true,			/* pc_relative */
	 5,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_RELBRANCH",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00ffffe0,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GET/SET offset */
  HOWTO (R_METAG_GETSETOFF,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GETSETOFF",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (6),
  EMPTY_HOWTO (7),
  EMPTY_HOWTO (8),
  EMPTY_HOWTO (9),
  EMPTY_HOWTO (10),
  EMPTY_HOWTO (11),
  EMPTY_HOWTO (12),
  EMPTY_HOWTO (13),
  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),
  EMPTY_HOWTO (16),
  EMPTY_HOWTO (17),
  EMPTY_HOWTO (18),
  EMPTY_HOWTO (19),
  EMPTY_HOWTO (20),
  EMPTY_HOWTO (21),
  EMPTY_HOWTO (22),
  EMPTY_HOWTO (23),
  EMPTY_HOWTO (24),
  EMPTY_HOWTO (25),
  EMPTY_HOWTO (26),
  EMPTY_HOWTO (27),
  EMPTY_HOWTO (28),
  EMPTY_HOWTO (29),

  HOWTO (R_METAG_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,		/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_METAG_GNU_VTINHERIT", /* name */
	 false,		/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_METAG_GNU_VTENTRY",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* High order 16 bit GOT offset */
  HOWTO (R_METAG_HI16_GOTOFF,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_HI16_GOTOFF", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low order 16 bit GOT offset */
  HOWTO (R_METAG_LO16_GOTOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_LO16_GOTOFF", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GET/SET GOT offset */
  HOWTO (R_METAG_GETSET_GOTOFF, /* type */
	 0,			/* rightshift */
	 2,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GETSET_GOTOFF", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GET/SET GOT relative */
  HOWTO (R_METAG_GETSET_GOT,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GETSET_GOT",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* High order 16 bit GOT reference */
  HOWTO (R_METAG_HI16_GOTPC,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_HI16_GOTPC",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low order 16 bit GOT reference */
  HOWTO (R_METAG_LO16_GOTPC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_LO16_GOTPC",  /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High order 16 bit PLT */
  HOWTO (R_METAG_HI16_PLT,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_HI16_PLT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low order 16 bit PLT */
  HOWTO (R_METAG_LO16_PLT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_LO16_PLT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_RELBRANCH_PLT, /* type */
	 2,			/* rightshift */
	 4,			/* size */
	 19,			/* bitsize */
	 true,			/* pc_relative */
	 5,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_RELBRANCH_PLT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00ffffe0,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Dummy relocs used by the linker internally.  */
  HOWTO (R_METAG_GOTOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GOTOFF",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_PLT,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GOTOFF",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* This is used only by the dynamic linker.  The symbol should exist
     both in the object being run and in some shared library.  The
     dynamic linker copies the data addressed by the symbol from the
     shared library into the object, because the object being
     run has to have the data at some particular address.  */
  HOWTO (R_METAG_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_COPY",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Marks a procedure linkage table entry for a symbol.  */
  HOWTO (R_METAG_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Used only by the dynamic linker.  When the object is run, this
     longword is set to the load address of the object, plus the
     addend.  */
  HOWTO (R_METAG_RELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_GD,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_GD",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LDM",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_LDO_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LDO_HI16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_LDO_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LDO_LO16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Dummy reloc used by the linker internally.  */
  HOWTO (R_METAG_TLS_LDO,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LDO",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_IE,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_IE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007ff80,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Dummy reloc used by the linker internally.  */
  HOWTO (R_METAG_TLS_IENONPIC,  /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_IENONPIC", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_IENONPIC_HI16,/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_IENONPIC_HI16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_IENONPIC_LO16,/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_IENONPIC_LO16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_TPOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_TPOFF",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_DTPMOD,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_DTPMOD",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_DTPOFF,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_DTPOFF",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Dummy reloc used by the linker internally.  */
  HOWTO (R_METAG_TLS_LE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_LE_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LE_HI16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_METAG_TLS_LE_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 3,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_METAG_TLS_LE_LO16", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0007fff8,		/* dst_mask */
	 false),		/* pcrel_offset */

};

#define BRANCH_BITS 19

/* The GOT is typically accessed using a [GS]ETD instruction. The size of the
   immediate offset which can be used in such instructions therefore limits
   the usable size of the GOT. If the base register for the [GS]ETD (A1LbP)
   is pointing to the base of the GOT then the size is limited to the maximum
   11 bits unsigned dword offset, or 2^13 = 0x2000 bytes. However the offset
   in a [GS]ETD instruction is signed, so by setting the base address register
   to an offset of that 0x2000 byte maximum unsigned offset from the base of
   the GOT we can use negative offsets in addition to positive. This
   effectively doubles the usable GOT size to 0x4000 bytes.  */
#define GOT_REG_OFFSET 0x2000

struct metag_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int metag_reloc_val;
};

static const struct metag_reloc_map metag_reloc_map [] =
  {
    { BFD_RELOC_NONE,		     R_METAG_NONE },
    { BFD_RELOC_32,		     R_METAG_ADDR32 },
    { BFD_RELOC_METAG_HIADDR16,	     R_METAG_HIADDR16 },
    { BFD_RELOC_METAG_LOADDR16,	     R_METAG_LOADDR16 },
    { BFD_RELOC_METAG_RELBRANCH,     R_METAG_RELBRANCH },
    { BFD_RELOC_METAG_GETSETOFF,     R_METAG_GETSETOFF },
    { BFD_RELOC_VTABLE_INHERIT,	     R_METAG_GNU_VTINHERIT },
    { BFD_RELOC_VTABLE_ENTRY,	     R_METAG_GNU_VTENTRY },
    { BFD_RELOC_METAG_REL8,	     R_METAG_REL8 },
    { BFD_RELOC_METAG_REL16,	     R_METAG_REL16 },
    { BFD_RELOC_METAG_HI16_GOTOFF,   R_METAG_HI16_GOTOFF },
    { BFD_RELOC_METAG_LO16_GOTOFF,   R_METAG_LO16_GOTOFF },
    { BFD_RELOC_METAG_GETSET_GOTOFF, R_METAG_GETSET_GOTOFF },
    { BFD_RELOC_METAG_GETSET_GOT,    R_METAG_GETSET_GOT },
    { BFD_RELOC_METAG_HI16_GOTPC,    R_METAG_HI16_GOTPC },
    { BFD_RELOC_METAG_LO16_GOTPC,    R_METAG_LO16_GOTPC },
    { BFD_RELOC_METAG_HI16_PLT,	     R_METAG_HI16_PLT },
    { BFD_RELOC_METAG_LO16_PLT,	     R_METAG_LO16_PLT },
    { BFD_RELOC_METAG_RELBRANCH_PLT, R_METAG_RELBRANCH_PLT },
    { BFD_RELOC_METAG_GOTOFF,	     R_METAG_GOTOFF },
    { BFD_RELOC_METAG_PLT,	     R_METAG_PLT },
    { BFD_RELOC_METAG_COPY,	     R_METAG_COPY },
    { BFD_RELOC_METAG_JMP_SLOT,	     R_METAG_JMP_SLOT },
    { BFD_RELOC_METAG_RELATIVE,	     R_METAG_RELATIVE },
    { BFD_RELOC_METAG_GLOB_DAT,	     R_METAG_GLOB_DAT },
    { BFD_RELOC_METAG_TLS_GD,	     R_METAG_TLS_GD },
    { BFD_RELOC_METAG_TLS_LDM,	     R_METAG_TLS_LDM },
    { BFD_RELOC_METAG_TLS_LDO_HI16,  R_METAG_TLS_LDO_HI16 },
    { BFD_RELOC_METAG_TLS_LDO_LO16,  R_METAG_TLS_LDO_LO16 },
    { BFD_RELOC_METAG_TLS_LDO,	     R_METAG_TLS_LDO },
    { BFD_RELOC_METAG_TLS_IE,	     R_METAG_TLS_IE },
    { BFD_RELOC_METAG_TLS_IENONPIC,  R_METAG_TLS_IENONPIC },
    { BFD_RELOC_METAG_TLS_IENONPIC_HI16, R_METAG_TLS_IENONPIC_HI16 },
    { BFD_RELOC_METAG_TLS_IENONPIC_LO16, R_METAG_TLS_IENONPIC_LO16 },
    { BFD_RELOC_METAG_TLS_TPOFF,     R_METAG_TLS_TPOFF },
    { BFD_RELOC_METAG_TLS_DTPMOD,    R_METAG_TLS_DTPMOD },
    { BFD_RELOC_METAG_TLS_DTPOFF,    R_METAG_TLS_DTPOFF },
    { BFD_RELOC_METAG_TLS_LE,	     R_METAG_TLS_LE },
    { BFD_RELOC_METAG_TLS_LE_HI16,   R_METAG_TLS_LE_HI16 },
    { BFD_RELOC_METAG_TLS_LE_LO16,   R_METAG_TLS_LE_LO16 },
  };

enum elf_metag_stub_type
{
  metag_stub_long_branch,
  metag_stub_long_branch_shared,
  metag_stub_none
};

struct elf_metag_stub_hash_entry
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

  enum elf_metag_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_metag_link_hash_entry *hh;

  /* And the reloc addend that this was derived from.  */
  bfd_vma addend;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;
};

struct elf_metag_link_hash_entry
{
  struct elf_link_hash_entry eh;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf_metag_stub_hash_entry *hsh_cache;

  enum
    {
      GOT_UNKNOWN = 0, GOT_NORMAL = 1, GOT_TLS_IE = 2, GOT_TLS_LDM = 4, GOT_TLS_GD = 8
    } tls_type;
};

struct elf_metag_link_hash_table
{
  /* The main hash table.  */
  struct elf_link_hash_table etab;

  /* The stub hash table.  */
  struct bfd_hash_table bstab;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);

  /* Array to keep track of which stub sections have been created, and
     information on stub grouping.  */
  struct map_stub
  {
    /* This is the section to which stubs in the group will be
       attached.  */
    asection *link_sec;
    /* The stub section.  */
    asection *stub_sec;
  } *stub_group;

  /* Assorted information used by elf_metag_size_stubs.  */
  unsigned int bfd_count;
  unsigned int top_index;
  asection **input_list;
  Elf_Internal_Sym **all_local_syms;

  /* Data for LDM relocations.  */
  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tls_ldm_got;
};

/* Return the base vma address which should be subtracted from the
   real address when resolving a dtpoff relocation.  This is PT_TLS
   segment p_vaddr.  */
static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the relocation value for R_METAG_TLS_IE */
static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  /* METAG TLS ABI is variant I and static TLS blocks start just after
     tcbhead structure which has 2 pointer fields.  */
  return (address - elf_hash_table (info)->tls_sec->vma
	  + align_power ((bfd_vma) 8,
			 elf_hash_table (info)->tls_sec->alignment_power));
}

static bool
metag_info_to_howto_rela (bfd *abfd,
			  arelent *cache_ptr,
			  Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_METAG_MAX)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = & elf_metag_howto_table [r_type];
  return true;
}

static reloc_howto_type *
metag_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (metag_reloc_map) / sizeof (metag_reloc_map[0]); i++)
    if (metag_reloc_map [i].bfd_reloc_val == code)
      return & elf_metag_howto_table [metag_reloc_map[i].metag_reloc_val];

  return NULL;
}

static reloc_howto_type *
metag_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			 const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (elf_metag_howto_table) / sizeof (elf_metag_howto_table[0]); i++)
    if (elf_metag_howto_table[i].name != NULL
	&& strcasecmp (elf_metag_howto_table[i].name, r_name) == 0)
      return &elf_metag_howto_table[i];

  return NULL;
}

/* Various hash macros and functions.  */
#define metag_link_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == METAG_ELF_DATA)	\
   ? (struct elf_metag_link_hash_table *) (p)->hash : NULL)

#define metag_elf_hash_entry(ent) \
  ((struct elf_metag_link_hash_entry *)(ent))

#define metag_stub_hash_entry(ent) \
  ((struct elf_metag_stub_hash_entry *)(ent))

#define metag_stub_hash_lookup(table, string, create, copy) \
  ((struct elf_metag_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

#define metag_elf_local_got_tls_type(abfd) \
  ((char *)(elf_local_got_offsets (abfd) + (elf_tdata (abfd)->symtab_hdr.sh_info)))

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
				 sizeof (struct elf_metag_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_metag_stub_hash_entry *hsh;

      /* Initialize the local fields.  */
      hsh = (struct elf_metag_stub_hash_entry *) entry;
      hsh->stub_sec = NULL;
      hsh->stub_offset = 0;
      hsh->target_value = 0;
      hsh->target_section = NULL;
      hsh->stub_type = metag_stub_long_branch;
      hsh->hh = NULL;
      hsh->id_sec = NULL;
    }

  return entry;
}

/* Initialize an entry in the link hash table.  */

static struct bfd_hash_entry *
metag_link_hash_newfunc (struct bfd_hash_entry *entry,
			 struct bfd_hash_table *table,
			 const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf_metag_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_metag_link_hash_entry *hh;

      /* Initialize the local fields.  */
      hh = (struct elf_metag_link_hash_entry *) entry;
      hh->hsh_cache = NULL;
      hh->tls_type = GOT_UNKNOWN;
    }

  return entry;
}

/* Free the derived linker hash table.  */

static void
elf_metag_link_hash_table_free (bfd *obfd)
{
  struct elf_metag_link_hash_table *htab
    = (struct elf_metag_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (&htab->bstab);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create the derived linker hash table.  The Meta ELF port uses the derived
   hash table to keep information specific to the Meta ELF linker (without
   using static variables).  */

static struct bfd_link_hash_table *
elf_metag_link_hash_table_create (bfd *abfd)
{
  struct elf_metag_link_hash_table *htab;
  size_t amt = sizeof (*htab);

  htab = bfd_zmalloc (amt);
  if (htab == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&htab->etab, abfd,
				      metag_link_hash_newfunc,
				      sizeof (struct elf_metag_link_hash_entry),
				      METAG_ELF_DATA))
    {
      free (htab);
      return NULL;
    }

  /* Init the stub hash table too.  */
  if (!bfd_hash_table_init (&htab->bstab, stub_hash_newfunc,
			    sizeof (struct elf_metag_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }
  htab->etab.root.hash_table_free = elf_metag_link_hash_table_free;
  htab->etab.dt_pltgot_required = true;

  return &htab->etab.root;
}

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

/* Build a name for an entry in the stub hash table.  */

static char *
metag_stub_name (const asection *input_section,
		 const asection *sym_sec,
		 const struct elf_metag_link_hash_entry *hh,
		 const Elf_Internal_Rela *rel)
{
  char *stub_name;
  bfd_size_type len;

  if (hh)
    {
      len = 8 + 1 + strlen (hh->eh.root.root.string) + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%s+%x",
		   input_section->id & 0xffffffff,
		   hh->eh.root.root.string,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%x:%x+%x",
		   input_section->id & 0xffffffff,
		   sym_sec->id & 0xffffffff,
		   (int) ELF32_R_SYM (rel->r_info) & 0xffffffff,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  return stub_name;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf_metag_stub_hash_entry *
metag_get_stub_entry (const asection *input_section,
		      const asection *sym_sec,
		      struct elf_metag_link_hash_entry *hh,
		      const Elf_Internal_Rela *rel,
		      struct elf_metag_link_hash_table *htab)
{
  struct elf_metag_stub_hash_entry *hsh;
  const asection *id_sec;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  id_sec = htab->stub_group[input_section->id].link_sec;

  if (hh != NULL && hh->hsh_cache != NULL
      && hh->hsh_cache->hh == hh
      && hh->hsh_cache->id_sec == id_sec)
    {
      hsh = hh->hsh_cache;
    }
  else
    {
      char *stub_name;

      stub_name = metag_stub_name (id_sec, sym_sec, hh, rel);
      if (stub_name == NULL)
	return NULL;

      hsh = metag_stub_hash_lookup (&htab->bstab,
				    stub_name, false, false);

      if (hh != NULL)
	hh->hsh_cache = hsh;

      free (stub_name);
    }

  return hsh;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf_metag_stub_hash_entry *
metag_add_stub (const char *stub_name,
		asection *section,
		struct elf_metag_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;
  struct elf_metag_stub_hash_entry *hsh;

  link_sec = htab->stub_group[section->id].link_sec;
  stub_sec = htab->stub_group[section->id].stub_sec;
  if (stub_sec == NULL)
    {
      stub_sec = htab->stub_group[link_sec->id].stub_sec;
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

	  stub_sec = (*htab->add_stub_section) (s_name, link_sec);
	  if (stub_sec == NULL)
	    return NULL;
	  htab->stub_group[link_sec->id].stub_sec = stub_sec;
	}
      htab->stub_group[section->id].stub_sec = stub_sec;
    }

  /* Enter this entry into the linker stub hash table.  */
  hsh = metag_stub_hash_lookup (&htab->bstab, stub_name,
				true, false);
  if (hsh == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner, stub_name);
      return NULL;
    }

  hsh->stub_sec = stub_sec;
  hsh->stub_offset = 0;
  hsh->id_sec = link_sec;
  return hsh;
}

/* Check a signed integer value can be represented in the given number
   of bits.  */

static bool
within_signed_range (int value, unsigned int bits)
{
  int min_val = -(1 << (bits - 1));
  int max_val = (1 << (bits - 1)) - 1;
  return (value <= max_val) && (value >= min_val);
}

/* Perform a relocation as part of a final link.  */

static bfd_reloc_status_type
metag_final_link_relocate (reloc_howto_type *howto,
			   bfd *input_bfd,
			   asection *input_section,
			   bfd_byte *contents,
			   Elf_Internal_Rela *rel,
			   bfd_vma relocation,
			   struct elf_metag_link_hash_entry *hh,
			   struct elf_metag_link_hash_table *htab,
			   asection *sym_sec)
{
  bfd_reloc_status_type r = bfd_reloc_ok;
  bfd_byte *hit_data = contents + rel->r_offset;
  int opcode, op_shift, op_extended, l1, l2;
  bfd_signed_vma srel, addend = rel->r_addend;
  struct elf_metag_stub_hash_entry *hsh = NULL;
  bfd_vma location;

  /* Find out where we are and where we're going.  */
  location = (rel->r_offset +
	      input_section->output_offset +
	      input_section->output_section->vma);

  switch (howto->type)
    {
    case R_METAG_RELBRANCH:
    case R_METAG_RELBRANCH_PLT:
      /* Make it a pc relative offset.  */
      relocation -= location;
      break;
    case R_METAG_TLS_GD:
    case R_METAG_TLS_IE:
      relocation -= elf_gp (input_section->output_section->owner);
      break;
    default:
      break;
    }

  switch (howto->type)
    {
    case R_METAG_RELBRANCH_PLT:
    case R_METAG_RELBRANCH:
      opcode = bfd_get_32 (input_bfd, hit_data);

      srel = (bfd_signed_vma) relocation;
      srel += addend;

      /* If the branch is out of reach, then redirect the
	 call to the local stub for this function.  */
      if (srel > ((1 << (BRANCH_BITS + 1)) - 1) ||
	  (srel < - (1 << (BRANCH_BITS + 1))))
	{
	  if (sym_sec == NULL)
	    break;

	  hsh = metag_get_stub_entry (input_section, sym_sec,
				      hh, rel, htab);
	  if (hsh == NULL)
	    return bfd_reloc_undefined;

	  /* Munge up the value and addend so that we call the stub
	     rather than the procedure directly.  */
	  srel = (hsh->stub_offset
		  + hsh->stub_sec->output_offset
		  + hsh->stub_sec->output_section->vma);
	  srel -= location;
	}

      srel = srel >> 2;

      if (!within_signed_range (srel, BRANCH_BITS))
	{
	  if (hh && hh->eh.root.type == bfd_link_hash_undefweak)
	    srel = 0;
	  else
	    return bfd_reloc_overflow;
	}

      opcode &= ~(0x7ffff << 5);
      opcode |= ((srel & 0x7ffff) << 5);

      bfd_put_32 (input_bfd, opcode, hit_data);
      break;
    case R_METAG_GETSETOFF:
    case R_METAG_GETSET_GOT:
    case R_METAG_GETSET_GOTOFF:
      opcode = bfd_get_32 (input_bfd, hit_data);

      srel = (bfd_signed_vma) relocation;
      srel += addend;

      /* Is this a standard or extended GET/SET?  */
      if ((opcode & 0xf0000000) == 0xa0000000)
	{
	  /* Extended GET/SET.  */
	  l1 = opcode & 0x2;
	  l2 = opcode & 0x4;
	  op_extended = 1;
	}
      else
	{
	  /* Standard GET/SET.  */
	  l1 = opcode & 0x01000000;
	  l2 = opcode & 0x04000000;
	  op_extended = 0;
	}

      /* Calculate the width of the GET/SET and how much we need to
	 shift the result by.  */
      if (l2)
	if (l1)
	  op_shift = 3;
	else
	  op_shift = 2;
      else
	if (l1)
	  op_shift = 1;
	else
	  op_shift = 0;

      /* GET/SET offsets are scaled by the width of the transfer.  */
      srel = srel >> op_shift;

      /* Extended GET/SET has signed 12 bits of offset, standard has
	 signed 6 bits.  */
      if (op_extended)
	{
	  if (!within_signed_range (srel, 12))
	    {
	      if (hh && hh->eh.root.type == bfd_link_hash_undefweak)
		srel = 0;
	      else
		return bfd_reloc_overflow;
	    }
	  opcode &= ~(0xfff << 7);
	  opcode |= ((srel & 0xfff) << 7);
	}
      else
	{
	  if (!within_signed_range (srel, 5))
	    {
	      if (hh && hh->eh.root.type == bfd_link_hash_undefweak)
		srel = 0;
	      else
		return bfd_reloc_overflow;
	    }
	  opcode &= ~(0x3f << 8);
	  opcode |= ((srel & 0x3f) << 8);
	}

      bfd_put_32 (input_bfd, opcode, hit_data);
      break;
    case R_METAG_TLS_GD:
    case R_METAG_TLS_LDM:
      opcode = bfd_get_32 (input_bfd, hit_data);

      if ((bfd_signed_vma)relocation < 0)
       {
	 /* sign extend immediate */
	 if ((opcode & 0xf2000001) == 0x02000000)
	  {
	    /* ADD De.e,Dx.r,#I16 */
	    /* set SE bit */
	    opcode |= (1 << 1);
	  } else
	    return bfd_reloc_overflow;
       }

      bfd_put_32 (input_bfd, opcode, hit_data);

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);
      break;
    default:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);
    }

  return r;
}

/* This is defined because R_METAG_NONE != 0...
   See RELOC_AGAINST_DISCARDED_SECTION for details.  */
#define METAG_RELOC_AGAINST_DISCARDED_SECTION(info, input_bfd, input_section, \
					      rel, relend, howto, contents) \
  {									\
    _bfd_clear_contents (howto, input_bfd, input_section,		\
			 contents, rel->r_offset);			\
									\
    if (bfd_link_relocatable (info)					\
	&& (input_section->flags & SEC_DEBUGGING))			\
      {									\
	/* Only remove relocations in debug sections since other	\
	   sections may require relocations.  */			\
	Elf_Internal_Shdr *rel_hdr;					\
									\
	rel_hdr = _bfd_elf_single_rel_hdr (input_section->output_section); \
									\
	/* Avoid empty output section.  */				\
	if (rel_hdr->sh_size > rel_hdr->sh_entsize)			\
	  {								\
	    rel_hdr->sh_size -= rel_hdr->sh_entsize;			\
	    rel_hdr = _bfd_elf_single_rel_hdr (input_section);		\
	    rel_hdr->sh_size -= rel_hdr->sh_entsize;			\
									\
	    memmove (rel, rel + 1, (relend - rel) * sizeof (*rel));	\
									\
	    input_section->reloc_count--;				\
	    relend--;							\
	    rel--;							\
	    continue;							\
	  }								\
      }									\
									\
    rel->r_info = R_METAG_NONE;						\
    rel->r_addend = 0;							\
    continue;								\
  }

/* Relocate a META ELF section.

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
elf_metag_relocate_section (bfd *output_bfd,
			    struct bfd_link_info *info,
			    bfd *input_bfd,
			    asection *input_section,
			    bfd_byte *contents,
			    Elf_Internal_Rela *relocs,
			    Elf_Internal_Sym *local_syms,
			    asection **local_sections)
{
  bfd_vma *local_got_offsets;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **eh_syms;
  struct elf_metag_link_hash_table *htab;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  asection *sreloc;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  eh_syms = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  htab = metag_link_hash_table (info);
  local_got_offsets = elf_local_got_offsets (input_bfd);

  sreloc = NULL;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_metag_link_hash_entry *hh;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type == R_METAG_GNU_VTINHERIT
	  || r_type == R_METAG_GNU_VTENTRY
	  || r_type == R_METAG_NONE)
	continue;

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = elf_metag_howto_table + ELF32_R_TYPE (rel->r_info);
      hh     = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  struct elf_link_hash_entry *eh;
	  bool unresolved_reloc, warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, eh_syms,
				   eh, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  name = eh->root.root.string;
	  hh = (struct elf_metag_link_hash_entry *) eh;
	}

      if (sec != NULL && discarded_section (sec))
	  METAG_RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
						 rel, relend, howto, contents);

      if (bfd_link_relocatable (info))
	continue;

      switch (r_type)
	{
	case R_METAG_ADDR32:
	case R_METAG_RELBRANCH:
	  if ((input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if ((bfd_link_pic (info)
	       && r_symndx != STN_UNDEF
	       && (input_section->flags & SEC_ALLOC) != 0
	       && !(r_type == R_METAG_RELBRANCH
		    && (hh == NULL || SYMBOL_CALLS_LOCAL (info, &hh->eh))))
	      || (!bfd_link_pic (info)
		  && hh != NULL
		  && hh->eh.dynindx != -1
		  && !hh->eh.non_got_ref
		  && ((hh->eh.def_dynamic
		       && !hh->eh.def_regular)
		      || hh->eh.root.type == bfd_link_hash_undefweak
		      || hh->eh.root.type == bfd_link_hash_undefined)))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      bfd_byte *loc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

	      sreloc = elf_section_data (input_section)->sreloc;
	      BFD_ASSERT (sreloc != NULL);

	      skip = false;
	      relocate = false;

	      outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							 info,
							 input_section,
							 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1)
		skip = true;
	      else if (outrel.r_offset == (bfd_vma) -2)
		skip = true, relocate = true;
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		{
		  memset (&outrel, 0, sizeof outrel);
		  outrel.r_info = ELF32_R_INFO (0, R_METAG_NONE);
		}
	      else if (r_type == R_METAG_RELBRANCH)
		{
		  BFD_ASSERT (hh != NULL && hh->eh.dynindx != -1);
		  outrel.r_info = ELF32_R_INFO (hh->eh.dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* h->dynindx may be -1 if this symbol was marked to
		     become local.  */
		  if (hh == NULL
		      || ((info->symbolic || hh->eh.dynindx == -1)
			  && hh->eh.def_regular))
		    {
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_METAG_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      BFD_ASSERT (hh->eh.dynindx != -1);
		      outrel.r_info = ELF32_R_INFO (hh->eh.dynindx, r_type);
		      outrel.r_addend = rel->r_addend;
		    }
		}

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count * sizeof(Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel,loc);
	      ++sreloc->reloc_count;

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }
	  break;

	case R_METAG_RELBRANCH_PLT:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  if (hh == NULL)
	    break;

	  if (hh->eh.forced_local)
	    break;

	  if (hh->eh.plt.offset == (bfd_vma) -1 || htab->etab.splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  relocation = (htab->etab.splt->output_section->vma
			+ htab->etab.splt->output_offset
			+ hh->eh.plt.offset);
	  break;
	case R_METAG_HI16_GOTPC:
	case R_METAG_LO16_GOTPC:
	  BFD_ASSERT (htab->etab.sgot != NULL);

	  relocation = (htab->etab.sgot->output_section->vma +
			htab->etab.sgot->output_offset);
	  relocation += GOT_REG_OFFSET;
	  relocation -= (input_section->output_section->vma
			 + input_section->output_offset
			 + rel->r_offset);
	  break;
	case R_METAG_HI16_GOTOFF:
	case R_METAG_LO16_GOTOFF:
	case R_METAG_GETSET_GOTOFF:
	  BFD_ASSERT (htab->etab.sgot != NULL);

	  relocation -= (htab->etab.sgot->output_section->vma +
			 htab->etab.sgot->output_offset);
	  relocation -= GOT_REG_OFFSET;
	  break;
	case R_METAG_GETSET_GOT:
	  {
	    bfd_vma off;
	    bool do_got = 0;

	    /* Relocation is to the entry for this symbol in the
	       global offset table.  */
	    if (hh != NULL)
	      {
		bool dyn;

		off = hh->eh.got.offset;
		dyn = htab->etab.dynamic_sections_created;
		if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						       bfd_link_pic (info),
						       &hh->eh))
		  {
		    /* If we aren't going to call finish_dynamic_symbol,
		       then we need to handle initialisation of the .got
		       entry and create needed relocs here.  Since the
		       offset must always be a multiple of 4, we use the
		       least significant bit to record whether we have
		       initialised it already.  */
		    if ((off & 1) != 0)
		      off &= ~1;
		    else
		      {
			hh->eh.got.offset |= 1;
			do_got = 1;
		      }
		  }
	      }
	    else
	      {
		/* Local symbol case.  */
		if (local_got_offsets == NULL)
		  abort ();

		off = local_got_offsets[r_symndx];

		/* The offset must always be a multiple of 4.  We use
		   the least significant bit to record whether we have
		   already generated the necessary reloc.  */
		if ((off & 1) != 0)
		  off &= ~1;
		else
		  {
		    local_got_offsets[r_symndx] |= 1;
		    do_got = 1;
		  }
	      }

	    if (do_got)
	      {
		if (bfd_link_pic (info))
		  {
		    /* Output a dynamic relocation for this GOT entry.
		       In this case it is relative to the base of the
		       object because the symbol index is zero.  */
		    Elf_Internal_Rela outrel;
		    bfd_byte *loc;
		    asection *s = htab->etab.srelgot;

		    outrel.r_offset = (off
				       + htab->etab.sgot->output_offset
				       + htab->etab.sgot->output_section->vma);
		    outrel.r_info = ELF32_R_INFO (0, R_METAG_RELATIVE);
		    outrel.r_addend = relocation;
		    loc = s->contents;
		    loc += s->reloc_count++ * sizeof (Elf32_External_Rela);
		    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		  }
		else
		  bfd_put_32 (output_bfd, relocation,
			      htab->etab.sgot->contents + off);
	      }

	    if (off >= (bfd_vma) -2)
	      abort ();

	    relocation = off - GOT_REG_OFFSET;
	  }
	  break;
	case R_METAG_TLS_GD:
	case R_METAG_TLS_IE:
	  {
	    /* XXXMJF There is room here for optimisations. For example
	       converting from GD->IE, etc.  */
	    bfd_vma off;
	    int indx;
	    char tls_type;

	    if (htab->etab.sgot == NULL)
	      abort();

	    indx = 0;
	    if (hh != NULL)
	      {
		bool dyn;
		dyn = htab->etab.dynamic_sections_created;

		if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     &hh->eh)
		    && (!bfd_link_pic (info)
			|| !SYMBOL_REFERENCES_LOCAL (info, &hh->eh)))
		  {
		    indx = hh->eh.dynindx;
		  }
		off = hh->eh.got.offset;
		tls_type = hh->tls_type;
	      }
	    else
	      {
		/* Local symbol case.  */
		if (local_got_offsets == NULL)
		  abort ();

		off = local_got_offsets[r_symndx];
		tls_type = metag_elf_local_got_tls_type (input_bfd) [r_symndx];
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
		    && (hh == NULL
			|| ELF_ST_VISIBILITY (hh->eh.other) == STV_DEFAULT
			|| hh->eh.root.type != bfd_link_hash_undefweak))
		  {
		    need_relocs = true;
		    loc = htab->etab.srelgot->contents;
		    /* FIXME (CAO): Should this be reloc_count++ ? */
		    loc += htab->etab.srelgot->reloc_count * sizeof (Elf32_External_Rela);
		  }

		if (tls_type & GOT_TLS_GD)
		  {
		    if (need_relocs)
		      {
			outrel.r_offset = (cur_off
					   + htab->etab.sgot->output_section->vma
					   + htab->etab.sgot->output_offset);
			outrel.r_info = ELF32_R_INFO (indx, R_METAG_TLS_DTPMOD);
			outrel.r_addend = 0;
			bfd_put_32 (output_bfd, 0, htab->etab.sgot->contents + cur_off);

			bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
			htab->etab.srelgot->reloc_count++;
			loc += sizeof (Elf32_External_Rela);

			if (indx == 0)
			  bfd_put_32 (output_bfd, 0,
				      htab->etab.sgot->contents + cur_off + 4);
			else
			  {
			    bfd_put_32 (output_bfd, 0,
					htab->etab.sgot->contents + cur_off + 4);
			    outrel.r_info = ELF32_R_INFO (indx,
						      R_METAG_TLS_DTPOFF);
			    outrel.r_offset += 4;
			    bfd_elf32_swap_reloca_out (output_bfd,
						       &outrel, loc);
			    htab->etab.srelgot->reloc_count++;
			    loc += sizeof (Elf32_External_Rela);
			  }
		      }
		    else
		      {
			/* We don't support changing the TLS model.  */
			/* PR 20675 */
			if (bfd_link_pic (info))
			  _bfd_error_handler (_("%pB(%pA): multiple TLS models are not supported"),
					      input_bfd, input_section);
			else
			  _bfd_error_handler (_("%pB(%pA): shared library symbol %s encountered whilst performing a static link"),
					      input_bfd, input_section, name);
			return false;
		      }

		    cur_off += 8;
		  }

		if (tls_type & GOT_TLS_IE)
		  {
		    if (need_relocs)
		      {
			outrel.r_offset = (cur_off
					   + htab->etab.sgot->output_section->vma
					   + htab->etab.sgot->output_offset);
			outrel.r_info = ELF32_R_INFO (indx, R_METAG_TLS_TPOFF);

			if (indx == 0)
			  outrel.r_addend = relocation - dtpoff_base (info);
			else
			  outrel.r_addend = 0;

			bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
			htab->etab.srelgot->reloc_count++;
			loc += sizeof (Elf32_External_Rela);
		      }
		    else
		      bfd_put_32 (output_bfd, tpoff (info, relocation),
				  htab->etab.sgot->contents + cur_off);

		    cur_off += 4;
		  }

		  if (hh != NULL)
		    hh->eh.got.offset |= 1;
		  else
		    local_got_offsets[r_symndx] |= 1;
	      }

	    /* Add the base of the GOT to the relocation value.  */
	    relocation = off - GOT_REG_OFFSET;

	    break;
	  }

	case R_METAG_TLS_IENONPIC_HI16:
	case R_METAG_TLS_IENONPIC_LO16:
	case R_METAG_TLS_LE_HI16:
	case R_METAG_TLS_LE_LO16:
	  if (bfd_link_pic (info))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA+%#" PRIx64 "): "
		   "%s relocation not permitted in shared object"),
		 input_bfd, input_section, (uint64_t) rel->r_offset,
		 howto->name);
	      return false;
	    }
	  else
	    relocation = tpoff (info, relocation);
	  break;
	case R_METAG_TLS_LDO_HI16:
	case R_METAG_TLS_LDO_LO16:
	  if (! bfd_link_pic (info))
	    relocation = tpoff (info, relocation);
	  else
	    relocation -= dtpoff_base (info);
	  break;
	case R_METAG_TLS_LDM:
	  {
	    bfd_vma off;

	    if (htab->etab.sgot == NULL)
	      abort();
	    off = htab->tls_ldm_got.offset;
	    if (off & 1)
	      off &= ~1;
	    else
	      {
		Elf_Internal_Rela outrel;
		bfd_byte *loc;

		outrel.r_offset = (off
				   + htab->etab.sgot->output_section->vma
				   + htab->etab.sgot->output_offset);

		outrel.r_addend = 0;
		outrel.r_info = ELF32_R_INFO (0, R_METAG_TLS_DTPMOD);
		loc = htab->etab.srelgot->contents;
		loc += htab->etab.srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		htab->tls_ldm_got.offset |= 1;
	      }

	    relocation = off - GOT_REG_OFFSET;
	    break;
	  }
	default:
	  break;
	}

      r = metag_final_link_relocate (howto, input_bfd, input_section,
				     contents, rel, relocation, hh, htab,
				     sec);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (hh ? &hh->eh.root : NULL), name, howto->name,
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

/* Create the .plt and .got sections, and set up our hash table
   short-cuts to various dynamic sections.  */

static bool
elf_metag_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_metag_link_hash_table *htab;
  struct elf_link_hash_entry *eh;
  struct bfd_link_hash_entry *bh;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  /* Don't try to create the .plt and .got twice.  */
  htab = metag_link_hash_table (info);
  if (htab->etab.splt != NULL)
    return true;

  /* Call the generic code to do most of the work.  */
  if (! _bfd_elf_create_dynamic_sections (abfd, info))
    return false;

  /* The header goes at the start of the dynamic .got section, which
     is placed after the dynamic .got.plt section.  ie. The header is
     not necessarily at the start of the output .got section.  */
  htab->etab.sgot->size += 12;

  /* Define the symbol __GLOBAL_OFFSET_TABLE__ on the header.  */
  bh = NULL;
  if (!(_bfd_generic_link_add_one_symbol
	(info, abfd, "__GLOBAL_OFFSET_TABLE__", BSF_GLOBAL, htab->etab.sgot,
	 (bfd_vma) 0, NULL, false, bed->collect, &bh)))
    return false;
  eh = (struct elf_link_hash_entry *) bh;
  eh->def_regular = 1;
  eh->type = STT_OBJECT;
  eh->other = STV_HIDDEN;

  if (! bfd_link_executable (info)
      && ! bfd_elf_link_record_dynamic_symbol (info, eh))
    return false;

  htab->etab.hgot = eh;

  return true;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure linkage
   table, and dynamic reloc sections.  At this point we haven't
   necessarily read all the input files.  */

static bool
elf_metag_check_relocs (bfd *abfd,
			struct bfd_link_info *info,
			asection *sec,
			const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **eh_syms;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  struct elf_metag_link_hash_table *htab;
  asection *sreloc;
  bfd *dynobj;
  int tls_type = GOT_UNKNOWN, old_tls_type = GOT_UNKNOWN;

  if (bfd_link_relocatable (info))
    return true;

  htab = metag_link_hash_table (info);
  dynobj = htab->etab.dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  eh_syms = elf_sym_hashes (abfd);
  sreloc = NULL;

  if (htab == NULL)
    return false;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      int r_type;
      struct elf_metag_link_hash_entry *hh;
      Elf_Internal_Sym *isym;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->etab.sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return false;

	  hh = NULL;
	}
      else
	{
	  isym = NULL;

	  hh = (struct elf_metag_link_hash_entry *)
	    eh_syms[r_symndx - symtab_hdr->sh_info];
	  while (hh->eh.root.type == bfd_link_hash_indirect
		 || hh->eh.root.type == bfd_link_hash_warning)
	    hh = (struct elf_metag_link_hash_entry *) hh->eh.root.u.i.link;
	}

      /* Some relocs require a global offset table.  */
      if (htab->etab.sgot == NULL)
	{
	  switch (r_type)
	    {
	    case R_METAG_TLS_GD:
	    case R_METAG_TLS_LDM:
	    case R_METAG_TLS_IE:
	      if (bfd_link_pic (info))
		info->flags |= DF_STATIC_TLS;
	      /* Fall through.  */

	    case R_METAG_HI16_GOTOFF:
	    case R_METAG_LO16_GOTOFF:
	    case R_METAG_GETSET_GOTOFF:
	    case R_METAG_GETSET_GOT:
	    case R_METAG_HI16_GOTPC:
	    case R_METAG_LO16_GOTPC:
	      if (dynobj == NULL)
		htab->etab.dynobj = dynobj = abfd;
	      if (!elf_metag_create_dynamic_sections (dynobj, info))
		return false;
	      break;

	    default:
	      break;
	    }
	}

      switch (r_type)
	{
	case R_METAG_TLS_IE:
	case R_METAG_TLS_GD:
	case R_METAG_GETSET_GOT:
	  switch (r_type)
	    {
	    default:
	      tls_type = GOT_NORMAL;
	      break;
	    case R_METAG_TLS_IE:
	      tls_type = GOT_TLS_IE;
	      break;
	    case R_METAG_TLS_GD:
	      tls_type = GOT_TLS_GD;
	      break;
	    }

	  if (hh != NULL)
	    {
	      hh->eh.got.refcount += 1;
	      old_tls_type = hh->tls_type;
	    }
	  else
	    {
	      bfd_signed_vma *local_got_refcounts;

	      /* This is a global offset table entry for a local
		 symbol.  */
	      local_got_refcounts = elf_local_got_refcounts (abfd);
	      if (local_got_refcounts == NULL)
		{
		  bfd_size_type size;

		  size = symtab_hdr->sh_info;
		  size *= sizeof (bfd_signed_vma);
		  /* Add in space to store the local GOT TLS types.  */
		  size += symtab_hdr->sh_info;
		  local_got_refcounts = ((bfd_signed_vma *)
					 bfd_zalloc (abfd, size));
		  if (local_got_refcounts == NULL)
		    return false;
		  elf_local_got_refcounts (abfd) = local_got_refcounts;
		  memset (metag_elf_local_got_tls_type (abfd),
			  GOT_UNKNOWN, symtab_hdr->sh_info);
		}
	      local_got_refcounts[r_symndx] += 1;
	      old_tls_type = metag_elf_local_got_tls_type (abfd) [r_symndx];
	    }

	  if (old_tls_type != tls_type)
	    {
	      if (hh != NULL)
		{
		  hh->tls_type = tls_type;
		}
	      else
		{
		  metag_elf_local_got_tls_type (abfd) [r_symndx] = tls_type;
		}
	    }

	  break;

	case R_METAG_TLS_LDM:
	  metag_link_hash_table (info)->tls_ldm_got.refcount += 1;
	  break;

	case R_METAG_RELBRANCH_PLT:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code without
	     linking in any dynamic objects, in which case we don't
	     need to generate a procedure linkage table after all.  */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (hh == NULL)
	    continue;

	  if (hh->eh.forced_local)
	    break;

	  hh->eh.needs_plt = 1;
	  hh->eh.plt.refcount += 1;
	  break;

	case R_METAG_HIADDR16:
	case R_METAG_LOADDR16:
	  /* Let's help debug shared library creation.  These relocs
	     cannot be used in shared libs.  Don't error out for
	     sections we don't care about, such as debug sections or
	     non-constant sections.  */
	  if (bfd_link_pic (info)
	      && (sec->flags & SEC_ALLOC) != 0
	      && (sec->flags & SEC_READONLY) != 0)
	    {
	      const char *name;

	      if (hh)
		name = hh->eh.root.root.string;
	      else
		name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: relocation %s against `%s' can not be used when making a shared object; recompile with -fPIC"),
		 abfd, elf_metag_howto_table[r_type].name, name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }

	  /* Fall through.  */
	case R_METAG_ADDR32:
	case R_METAG_RELBRANCH:
	case R_METAG_GETSETOFF:
	  if (hh != NULL && !bfd_link_pic (info))
	    {
	      hh->eh.non_got_ref = 1;
	      hh->eh.plt.refcount += 1;
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
	     later (it is never cleared).  We account for that
	     possibility below by storing information in the
	     dyn_relocs field of the hash table entry. A similar
	     situation occurs when creating shared libraries and symbol
	     visibility changes render the symbol local.

	     If on the other hand, we are creating an executable, we
	     may need to keep relocations for symbols satisfied by a
	     dynamic library if we manage to avoid copy relocs for the
	     symbol.  */
	  if ((bfd_link_pic (info)
	       && (sec->flags & SEC_ALLOC) != 0
	       && (r_type != R_METAG_RELBRANCH
		   || (hh != NULL
		       && (! info->symbolic
			   || hh->eh.root.type == bfd_link_hash_defweak
			   || !hh->eh.def_regular))))
	      || (!bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && hh != NULL
		  && (hh->eh.root.type == bfd_link_hash_defweak
		      || !hh->eh.def_regular)))
	    {
	      struct elf_dyn_relocs *hdh_p;
	      struct elf_dyn_relocs **hdh_head;

	      if (dynobj == NULL)
		htab->etab.dynobj = dynobj = abfd;

	      /* When creating a shared object, we must copy these
		 relocs into the output file.  We create a reloc
		 section in dynobj and make room for the reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->etab.dynobj, 2, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    {
		      bfd_set_error (bfd_error_bad_value);
		      return false;
		    }

		  elf_section_data (sec)->sreloc = sreloc;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (hh != NULL)
		hdh_head = &hh->eh.dyn_relocs;
	      else
		{
		  /* Track dynamic relocs needed for local syms too.  */
		  asection *sr;
		  void *vpp;

		  sr = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (sr == NULL)
		    sr = sec;

		  vpp = &elf_section_data (sr)->local_dynrel;
		  hdh_head = (struct elf_dyn_relocs **) vpp;
		}

	      hdh_p = *hdh_head;
	      if (hdh_p == NULL || hdh_p->sec != sec)
		{
		  hdh_p = ((struct elf_dyn_relocs *)
			   bfd_alloc (dynobj, sizeof *hdh_p));
		  if (hdh_p == NULL)
		    return false;
		  hdh_p->next = *hdh_head;
		  *hdh_head = hdh_p;
		  hdh_p->sec = sec;
		  hdh_p->count = 0;
		  hdh_p->pc_count = 0;
		}

	      hdh_p->count += 1;
	      if (ELF32_R_TYPE (rel->r_info) == R_METAG_RELBRANCH)
		hdh_p->pc_count += 1;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_METAG_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, &hh->eh,
					    rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_METAG_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, &hh->eh, rel->r_addend))
	    return false;
	  break;
	}
    }

  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elf_metag_copy_indirect_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *eh_dir,
				struct elf_link_hash_entry *eh_ind)
{
  struct elf_metag_link_hash_entry *hh_dir, *hh_ind;

  hh_dir = metag_elf_hash_entry (eh_dir);
  hh_ind = metag_elf_hash_entry (eh_ind);

  if (eh_ind->root.type == bfd_link_hash_indirect
      && eh_dir->got.refcount <= 0)
    {
      hh_dir->tls_type = hh_ind->tls_type;
      hh_ind->tls_type = GOT_UNKNOWN;
    }

  _bfd_elf_link_hash_copy_indirect (info, eh_dir, eh_ind);
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf_metag_adjust_dynamic_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *eh)
{
  struct elf_metag_link_hash_table *htab;
  asection *s, *srel;

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (eh->type == STT_FUNC
      || eh->needs_plt)
    {
      if (eh->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, eh)
	  || (ELF_ST_VISIBILITY (eh->other) != STV_DEFAULT
	      && eh->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a PCREL
	     reloc instead.  */
	  eh->plt.offset = (bfd_vma) -1;
	  eh->needs_plt = 0;
	}

      return true;
    }
  else
    eh->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (eh->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (eh);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      eh->root.u.def.section = def->root.u.def.section;
      eh->root.u.def.value = def->root.u.def.value;
      eh->non_got_ref = def->non_got_ref;
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
  if (!eh->non_got_ref)
    return true;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      eh->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (!_bfd_elf_readonly_dynrelocs (eh))
    {
      eh->non_got_ref = 0;
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

  htab = metag_link_hash_table (info);

  /* We must generate a COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((eh->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->etab.sdynrelro;
      srel = htab->etab.sreldynrelro;
    }
  else
    {
      s = htab->etab.sdynbss;
      srel = htab->etab.srelbss;
    }
  if ((eh->root.u.def.section->flags & SEC_ALLOC) != 0 && eh->size != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      eh->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, eh, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   global syms.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *eh, void *inf)
{
  struct bfd_link_info *info;
  struct elf_metag_link_hash_table *htab;
  struct elf_dyn_relocs *hdh_p;

  if (eh->root.type == bfd_link_hash_indirect)
    return true;

  if (eh->root.type == bfd_link_hash_warning)
    eh = (struct elf_link_hash_entry *) eh->root.u.i.link;

  info = inf;
  htab = metag_link_hash_table (info);

  if (htab->etab.dynamic_sections_created
      && eh->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (eh->dynindx == -1
	  && !eh->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, eh))
	    return false;
	}

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), eh))
	{
	  asection *s = htab->etab.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += PLT_ENTRY_SIZE;

	  eh->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info)
	      && !eh->def_regular)
	    {
	      eh->root.u.def.section = s;
	      eh->root.u.def.value = eh->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->etab.sgotplt->size += 4;

	  /* We also need to make an entry in the .rel.plt section.  */
	  htab->etab.srelplt->size += sizeof (Elf32_External_Rela);
	}
      else
	{
	  eh->plt.offset = (bfd_vma) -1;
	  eh->needs_plt = 0;
	}
    }
  else
    {
      eh->plt.offset = (bfd_vma) -1;
      eh->needs_plt = 0;
    }

  if (eh->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = metag_elf_hash_entry (eh)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (eh->dynindx == -1
	  && !eh->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, eh))
	    return false;
	}

      s = htab->etab.sgot;

      eh->got.offset = s->size;
      s->size += 4;
      /* R_METAG_TLS_GD needs 2 consecutive GOT slots.  */
      if (tls_type == GOT_TLS_GD)
	  s->size += 4;
      dyn = htab->etab.dynamic_sections_created;
      /* R_METAG_TLS_IE needs one dynamic relocation if dynamic,
	 R_METAG_TLS_GD needs one if local symbol and two if global.  */
      if ((tls_type == GOT_TLS_GD && eh->dynindx == -1)
	  || (tls_type == GOT_TLS_IE && dyn))
	htab->etab.srelgot->size += sizeof (Elf32_External_Rela);
      else if (tls_type == GOT_TLS_GD)
	  htab->etab.srelgot->size += 2 * sizeof (Elf32_External_Rela);
      else if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						bfd_link_pic (info),
						eh))
	  htab->etab.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    eh->got.offset = (bfd_vma) -1;

  if (eh->dyn_relocs == NULL)
    return true;

  /* If this is a -Bsymbolic shared link, then we need to discard all
     space allocated for dynamic pc-relative relocs against symbols
     defined in a regular object.  For the normal shared case, discard
     space for relocs that have become local due to symbol visibility
     changes.  */
  if (bfd_link_pic (info))
    {
      if (SYMBOL_CALLS_LOCAL (info, eh))
	{
	  struct elf_dyn_relocs **hdh_pp;

	  for (hdh_pp = &eh->dyn_relocs; (hdh_p = *hdh_pp) != NULL; )
	    {
	      hdh_p->count -= hdh_p->pc_count;
	      hdh_p->pc_count = 0;
	      if (hdh_p->count == 0)
		*hdh_pp = hdh_p->next;
	      else
		hdh_pp = &hdh_p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (eh->dyn_relocs != NULL
	  && eh->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (eh->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (eh->dynindx == -1
		   && !eh->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, eh))
		return false;
	    }
	}
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */
      if (!eh->non_got_ref
	  && ((eh->def_dynamic
	       && !eh->def_regular)
	      || (htab->etab.dynamic_sections_created
		  && (eh->root.type == bfd_link_hash_undefweak
		      || eh->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (eh->dynindx == -1
	      && !eh->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, eh))
		return false;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (eh->dynindx != -1)
	    goto keep;
	}

      eh->dyn_relocs = NULL;
      return true;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (hdh_p = eh->dyn_relocs; hdh_p != NULL; hdh_p = hdh_p->next)
    {
      asection *sreloc = elf_section_data (hdh_p->sec)->sreloc;
      sreloc->size += hdh_p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
elf_metag_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info)
{
  struct elf_metag_link_hash_table *htab;
  bfd *dynobj;
  bfd *ibfd;
  asection *s;
  bool relocs;

  htab = metag_link_hash_table (info);
  dynobj = htab->etab.dynobj;
  if (dynobj == NULL)
    abort ();

  if (htab->etab.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
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
      asection *srel;
      char *local_tls_type;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *hdh_p;

	  for (hdh_p = ((struct elf_dyn_relocs *)
			elf_section_data (s)->local_dynrel);
	       hdh_p != NULL;
	       hdh_p = hdh_p->next)
	    {
	      if (!bfd_is_abs_section (hdh_p->sec)
		  && bfd_is_abs_section (hdh_p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (hdh_p->count != 0)
		{
		  srel = elf_section_data (hdh_p->sec)->sreloc;
		  srel->size += hdh_p->count * sizeof (Elf32_External_Rela);
		  if ((hdh_p->sec->output_section->flags & SEC_READONLY) != 0)
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
      local_tls_type = metag_elf_local_got_tls_type (ibfd);
      s = htab->etab.sgot;
      srel = htab->etab.srelgot;
      for (; local_got < end_local_got; ++local_got)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += GOT_ENTRY_SIZE;
	      /* R_METAG_TLS_GD relocs need 2 consecutive GOT entries.  */
	      if (*local_tls_type == GOT_TLS_GD)
		s->size += 4;
	      if (bfd_link_pic (info))
		srel->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	  ++local_tls_type;
	}
    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate 2 got entries and 1 dynamic reloc for R_METAG_TLS_LDM
	 reloc.  */
      htab->tls_ldm_got.offset = htab->etab.sgot->size;
      htab->etab.sgot->size += 8;
      htab->etab.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->etab, allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      bool reloc_section = false;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->etab.splt
	  || s == htab->etab.sgot
	  || s == htab->etab.sgotplt
	  || s == htab->etab.sdynbss
	  || s == htab->etab.sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0 && s != htab->etab.srelplt)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	  reloc_section = true;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
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

      /* Allocate memory for the section contents.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
      else if (reloc_section)
	{
	  unsigned char *contents = s->contents;
	  Elf32_External_Rela reloc;

	  /* Fill the reloc section with a R_METAG_NONE type reloc.  */
	  memset(&reloc, 0, sizeof(Elf32_External_Rela));
	  reloc.r_info[0] = R_METAG_NONE;
	  for (; contents < (s->contents + s->size);
	       contents += sizeof(Elf32_External_Rela))
	    {
	      memcpy(contents, &reloc, sizeof(Elf32_External_Rela));
	    }
	}
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf_metag_finish_dynamic_symbol (bfd *output_bfd,
				 struct bfd_link_info *info,
				 struct elf_link_hash_entry *eh,
				 Elf_Internal_Sym *sym)
{
  struct elf_metag_link_hash_table *htab;
  Elf_Internal_Rela rel;
  bfd_byte *loc;

  htab = metag_link_hash_table (info);

  if (eh->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgot;
      asection *srela;

      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma got_entry;

      if (eh->plt.offset & 1)
	abort ();

      BFD_ASSERT (eh->dynindx != -1);

      splt = htab->etab.splt;
      sgot = htab->etab.sgotplt;
      srela = htab->etab.srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = eh->plt.offset / PLT_ENTRY_SIZE - 1;

      /* Get the offset into the .got.plt table of the entry that
	 corresponds to this function.  */
      got_offset = plt_index * GOT_ENTRY_SIZE;

      BFD_ASSERT (got_offset < (1 << 16));

      got_entry = sgot->output_section->vma
	+ sgot->output_offset
	+ got_offset;

      BFD_ASSERT (plt_index < (1 << 16));

      /* Fill in the entry in the procedure linkage table.  */
      if (! bfd_link_pic (info))
	{
	  bfd_put_32 (output_bfd,
		      (plt_entry[0]
		       | (((got_entry >> 16) & 0xffff) << 3)),
		      splt->contents + eh->plt.offset);
	  bfd_put_32 (output_bfd,
		      (plt_entry[1]
		       | ((got_entry & 0xffff) << 3)),
		      splt->contents + eh->plt.offset + 4);
	  bfd_put_32 (output_bfd, plt_entry[2],
		      splt->contents + eh->plt.offset + 8);
	  bfd_put_32 (output_bfd,
		      (plt_entry[3] | (plt_index << 3)),
		      splt->contents + eh->plt.offset + 12);
	  bfd_put_32 (output_bfd,
		      (plt_entry[4]
		       | ((((unsigned int) ((- (eh->plt.offset + 16)) >> 2)) & 0x7ffff) << 5)),
		      splt->contents + eh->plt.offset + 16);
	}
      else
	{
	  bfd_vma addr = got_entry - (splt->output_section->vma +
				      splt->output_offset + eh->plt.offset);

	  bfd_put_32 (output_bfd,
		      plt_pic_entry[0] | (((addr >> 16) & 0xffff) << 3),
		      splt->contents + eh->plt.offset);
	  bfd_put_32 (output_bfd,
		      plt_pic_entry[1] | ((addr & 0xffff) << 3),
		      splt->contents + eh->plt.offset + 4);
	  bfd_put_32 (output_bfd, plt_pic_entry[2],
		      splt->contents + eh->plt.offset + 8);
	  bfd_put_32 (output_bfd,
		      (plt_pic_entry[3] | (plt_index << 3)),
		      splt->contents + eh->plt.offset + 12);
	  bfd_put_32 (output_bfd,
		      (plt_pic_entry[4]
		       + ((((unsigned int) ((- (eh->plt.offset + 16)) >> 2)) & 0x7ffff) << 5)),
		      splt->contents + eh->plt.offset + 16);
	}

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset
		   + eh->plt.offset
		   + 12), /* offset within PLT entry */
		  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rel.r_offset = (sgot->output_section->vma
		      + sgot->output_offset
		      + got_offset);
      rel.r_info = ELF32_R_INFO (eh->dynindx, R_METAG_JMP_SLOT);
      rel.r_addend = 0;
      loc = htab->etab.srelplt->contents;
      loc += plt_index * sizeof(Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

      if (!eh->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }

  if (eh->got.offset != (bfd_vma) -1
      && (metag_elf_hash_entry (eh)->tls_type & GOT_TLS_GD) == 0
      && (metag_elf_hash_entry (eh)->tls_type & GOT_TLS_IE) == 0)
    {
      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      rel.r_offset = ((eh->got.offset &~ (bfd_vma) 1)
		      + htab->etab.sgot->output_offset
		      + htab->etab.sgot->output_section->vma);

      /* If this is a -Bsymbolic link and the symbol is defined
	 locally or was forced to be local because of a version file,
	 we just want to emit a RELATIVE reloc.  The entry in the
	 global offset table will already have been initialized in the
	 relocate_section function.  */
      if (bfd_link_pic (info)
	  && (info->symbolic || eh->dynindx == -1)
	  && eh->def_regular)
	{
	  rel.r_info = ELF32_R_INFO (0, R_METAG_RELATIVE);
	  rel.r_addend = (eh->root.u.def.value
			  + eh->root.u.def.section->output_offset
			  + eh->root.u.def.section->output_section->vma);
	}
      else
	{
	  if ((eh->got.offset & 1) != 0)
	    abort ();
	  bfd_put_32 (output_bfd, 0, htab->etab.sgot->contents + eh->got.offset);
	  rel.r_info = ELF32_R_INFO (eh->dynindx, R_METAG_GLOB_DAT);
	  rel.r_addend = 0;
	}

      loc = htab->etab.srelgot->contents;
      loc += htab->etab.srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  if (eh->needs_copy)
    {
      asection *s;

      /* This symbol needs a copy reloc.  Set it up.  */

      if (! (eh->dynindx != -1
	     && (eh->root.type == bfd_link_hash_defined
		 || eh->root.type == bfd_link_hash_defweak)))
	abort ();

      rel.r_offset = (eh->root.u.def.value
		      + eh->root.u.def.section->output_offset
		      + eh->root.u.def.section->output_section->vma);
      rel.r_addend = 0;
      rel.r_info = ELF32_R_INFO (eh->dynindx, R_METAG_COPY);
      if (eh->root.u.def.section == htab->etab.sdynrelro)
	s = htab->etab.sreldynrelro;
      else
	s = htab->etab.srelbss;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (eh->root.root.string[0] == '_'
      && (strcmp (eh->root.root.string, "_DYNAMIC") == 0
	  || eh == htab->etab.hgot))
    {
      sym->st_shndx = SHN_ABS;
    }

  return true;
}

/* Set the Meta ELF ABI version.  */

static bool
elf_metag_init_file_header (bfd *abfd, struct bfd_link_info *link_info)
{
  Elf_Internal_Ehdr * i_ehdrp;	/* ELF file header, internal form.  */

  if (!_bfd_elf_init_file_header (abfd, link_info))
    return false;

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_ABIVERSION] = METAG_ELF_ABI_VERSION;
  return true;
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf_metag_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    const asection *rel_sec ATTRIBUTE_UNUSED,
			    const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_METAG_RELATIVE:
      return reloc_class_relative;
    case R_METAG_JMP_SLOT:
      return reloc_class_plt;
    case R_METAG_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bool
elf_metag_finish_dynamic_sections (bfd *output_bfd,
				   struct bfd_link_info *info)
{
  bfd *dynobj;
  struct elf_metag_link_hash_table *htab;
  asection *sdyn;

  htab = metag_link_hash_table (info);
  dynobj = htab->etab.dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->etab.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      if (sdyn == NULL)
	abort ();

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
	      s = htab->etab.sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_JMPREL:
	      s = htab->etab.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = htab->etab.srelplt;
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }

	}

      /* Fill in the first entry in the procedure linkage table.  */
      splt = htab->etab.splt;
      if (splt && splt->size > 0)
	{
	  unsigned long addr;
	  /* addr = .got + 4 */
	  addr = (htab->etab.sgot->output_section->vma
		  + htab->etab.sgot->output_offset + 4);
	  if (bfd_link_pic (info))
	    {
	      addr -= splt->output_section->vma + splt->output_offset;
	      bfd_put_32 (output_bfd,
			  plt0_pic_entry[0] | (((addr >> 16) & 0xffff) << 3),
			  splt->contents);
	      bfd_put_32 (output_bfd,
			  plt0_pic_entry[1] | ((addr & 0xffff) << 3),
			  splt->contents + 4);
	      bfd_put_32 (output_bfd, plt0_pic_entry[2], splt->contents + 8);
	      bfd_put_32 (output_bfd, plt0_pic_entry[3], splt->contents + 12);
	      bfd_put_32 (output_bfd, plt0_pic_entry[4], splt->contents + 16);
	    }
	  else
	    {
	      bfd_put_32 (output_bfd,
			  plt0_entry[0] | (((addr >> 16) & 0xffff) << 3),
			  splt->contents);
	      bfd_put_32 (output_bfd,
			  plt0_entry[1] | ((addr & 0xffff) << 3),
			  splt->contents + 4);
	      bfd_put_32 (output_bfd, plt0_entry[2], splt->contents + 8);
	      bfd_put_32 (output_bfd, plt0_entry[3], splt->contents + 12);
	      bfd_put_32 (output_bfd, plt0_entry[4], splt->contents + 16);
	    }

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize =
	    PLT_ENTRY_SIZE;
	}
    }

  if (htab->etab.sgot != NULL && htab->etab.sgot->size != 0)
    {
      /* Fill in the first entry in the global offset table.
	 We use it to point to our dynamic section, if we have one.  */
      bfd_put_32 (output_bfd,
		  sdyn ? sdyn->output_section->vma + sdyn->output_offset : 0,
		  htab->etab.sgot->contents);

      /* The second entry is reserved for use by the dynamic linker.  */
      memset (htab->etab.sgot->contents + GOT_ENTRY_SIZE, 0, GOT_ENTRY_SIZE);

      /* Set .got entry size.  */
      elf_section_data (htab->etab.sgot->output_section)
	->this_hdr.sh_entsize = GOT_ENTRY_SIZE;
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
elf_metag_gc_mark_hook (asection *sec,
			struct bfd_link_info *info,
			Elf_Internal_Rela *rela,
			struct elf_link_hash_entry *hh,
			Elf_Internal_Sym *sym)
{
  if (hh != NULL)
    switch ((unsigned int) ELF32_R_TYPE (rela->r_info))
      {
      case R_METAG_GNU_VTINHERIT:
      case R_METAG_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rela, hh, sym);
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf_metag_stub_type
metag_type_of_stub (asection *input_sec,
		    const Elf_Internal_Rela *rel,
		    struct elf_metag_link_hash_entry *hh,
		    bfd_vma destination,
		    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  bfd_vma location;
  bfd_vma branch_offset;
  bfd_vma max_branch_offset;

  if (hh != NULL &&
      !(hh->eh.root.type == bfd_link_hash_defined
	|| hh->eh.root.type == bfd_link_hash_defweak))
    return metag_stub_none;

  /* Determine where the call point is.  */
  location = (input_sec->output_offset
	      + input_sec->output_section->vma
	      + rel->r_offset);

  branch_offset = destination - location;

  /* Determine if a long branch stub is needed.  Meta branch offsets
     are signed 19 bits 4 byte aligned.  */
  max_branch_offset = (1 << (BRANCH_BITS-1)) << 2;

  if (branch_offset + max_branch_offset >= 2*max_branch_offset)
    {
      if (bfd_link_pic (info))
	return metag_stub_long_branch_shared;
      else
	return metag_stub_long_branch;
    }

  return metag_stub_none;
}

#define MOVT_A0_3	0x82180005
#define JUMP_A0_3	0xac180003

#define MOVT_A1LBP	0x83080005
#define ADD_A1LBP	0x83080000

#define ADDT_A0_3_CPC	0x82980001
#define ADD_A0_3_A0_3	0x82180000
#define MOV_PC_A0_3	0xa3180ca0

static bool
metag_build_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct elf_metag_stub_hash_entry *hsh;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_byte *loc;
  bfd_vma sym_value;
  int size;
  struct bfd_link_info *info;

  /* Massage our args to the form they really have.  */
  hsh = (struct elf_metag_stub_hash_entry *) gen_entry;
  info = (struct bfd_link_info *) in_arg;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (hsh->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    hsh->target_section);

  stub_sec = hsh->stub_sec;

  /* Make a note of the offset within the stubs for this entry.  */
  hsh->stub_offset = stub_sec->size;
  loc = stub_sec->contents + hsh->stub_offset;

  stub_bfd = stub_sec->owner;

  switch (hsh->stub_type)
    {
    case metag_stub_long_branch_shared:
      /* A PIC long branch stub is an ADDT and an ADD instruction used to
	 calculate the jump target using A0.3 as a temporary. Then a MOV
	 to PC carries out the jump.  */
      sym_value = (hsh->target_value
		   + hsh->target_section->output_offset
		   + hsh->target_section->output_section->vma
		   + hsh->addend);

      sym_value -= (hsh->stub_offset
		    + stub_sec->output_offset
		    + stub_sec->output_section->vma);

      bfd_put_32 (stub_bfd, ADDT_A0_3_CPC | (((sym_value >> 16) & 0xffff) << 3),
		  loc);

      bfd_put_32 (stub_bfd, ADD_A0_3_A0_3 | ((sym_value & 0xffff) << 3),
		  loc + 4);

      bfd_put_32 (stub_bfd, MOV_PC_A0_3, loc + 8);

      size = 12;
      break;
    case metag_stub_long_branch:
      /* A standard long branch stub is a MOVT instruction followed by a
	 JUMP instruction using the A0.3 register as a temporary. This is
	 the same method used by the LDLK linker (patch.c).  */
      sym_value = (hsh->target_value
		   + hsh->target_section->output_offset
		   + hsh->target_section->output_section->vma
		   + hsh->addend);

      bfd_put_32 (stub_bfd, MOVT_A0_3 | (((sym_value >> 16) & 0xffff) << 3),
		  loc);

      bfd_put_32 (stub_bfd, JUMP_A0_3 | ((sym_value & 0xffff) << 3), loc + 4);

      size = 8;
      break;
    default:
      BFD_FAIL ();
      return false;
    }

  stub_sec->size += size;
  return true;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bool
metag_size_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg ATTRIBUTE_UNUSED)
{
  struct elf_metag_stub_hash_entry *hsh;
  int size = 0;

  /* Massage our args to the form they really have.  */
  hsh = (struct elf_metag_stub_hash_entry *) gen_entry;

  if (hsh->stub_type == metag_stub_long_branch)
    size = 8;
  else if (hsh->stub_type == metag_stub_long_branch_shared)
    size = 12;

  hsh->stub_sec->size += size;
  return true;
}

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf_metag_setup_section_lists (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  unsigned int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  size_t amt;
  struct elf_metag_link_hash_table *htab = metag_link_hash_table (info);

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
elf_metag_next_input_section (struct bfd_link_info *info, asection *isec)
{
  struct elf_metag_link_hash_table *htab = metag_link_hash_table (info);

  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;
      if (*list != bfd_abs_section_ptr)
	{
	  /* Steal the link_sec pointer for our list.  */
#define PREV_SEC(sec) (htab->stub_group[(sec)->id].link_sec)
	  /* This happens to make the list in reverse order,
	     which is what we want.  */
	  PREV_SEC (isec) = *list;
	  *list = isec;
	}
    }
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the beginning of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static void
group_sections (struct elf_metag_link_hash_table *htab,
		bfd_size_type stub_group_size,
		bool stubs_always_before_branch)
{
  asection **list = htab->input_list + htab->top_index;
  do
    {
      asection *tail = *list;
      if (tail == bfd_abs_section_ptr)
	continue;
      while (tail != NULL)
	{
	  asection *curr;
	  asection *prev;
	  bfd_size_type total;
	  bool big_sec;

	  curr = tail;
	  total = tail->size;
	  big_sec = total >= stub_group_size;

	  while ((prev = PREV_SEC (curr)) != NULL
		 && ((total += curr->output_offset - prev->output_offset)
		     < stub_group_size))
	    curr = prev;

	  /* OK, the size from the start of CURR to the end is less
	     than stub_group_size bytes and thus can be handled by one stub
	     section.  (or the tail section is itself larger than
	     stub_group_size bytes, in which case we may be toast.)
	     We should really be keeping track of the total size of
	     stubs added here, as stubs contribute to the final output
	     section size.  */
	  do
	    {
	      prev = PREV_SEC (tail);
	      /* Set up this stub group.  */
	      htab->stub_group[tail->id].link_sec = curr;
	    }
	  while (tail != curr && (tail = prev) != NULL);

	  /* But wait, there's more!  Input sections up to stub_group_size
	     bytes before the stub section can be handled by it too.
	     Don't do this if we have a really large section after the
	     stubs, as adding more stubs increases the chance that
	     branches may not reach into the stub section.  */
	  if (!stubs_always_before_branch && !big_sec)
	    {
	      total = 0;
	      while (prev != NULL
		     && ((total += tail->output_offset - prev->output_offset)
			 < stub_group_size))
		{
		  tail = prev;
		  prev = PREV_SEC (tail);
		  htab->stub_group[tail->id].link_sec = curr;
		}
	    }
	  tail = prev;
	}
    }
  while (list-- != htab->input_list);
  free (htab->input_list);
#undef PREV_SEC
}

/* Read in all local syms for all input bfds.
   Returns -1 on error, 0 otherwise.  */

static int
get_local_syms (bfd *output_bfd ATTRIBUTE_UNUSED, bfd *input_bfd,
		struct bfd_link_info *info)
{
  unsigned int bfd_indx;
  Elf_Internal_Sym *local_syms, **all_local_syms;
  int stub_changed = 0;
  struct elf_metag_link_hash_table *htab = metag_link_hash_table (info);

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

  return stub_changed;
}

/* Determine and set the size of the stub section for a final link.

The basic idea here is to examine all the relocations looking for
PC-relative calls to a target that is unreachable with a "CALLR"
instruction.  */

/* See elf32-hppa.c and elf64-ppc.c.  */

bool
elf_metag_size_stubs(bfd *output_bfd, bfd *stub_bfd,
		     struct bfd_link_info *info,
		     bfd_signed_vma group_size,
		     asection * (*add_stub_section) (const char *, asection *),
		     void (*layout_sections_again) (void))
{
  bfd_size_type stub_group_size;
  bool stubs_always_before_branch;
  bool stub_changed;
  struct elf_metag_link_hash_table *htab = metag_link_hash_table (info);

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;
  stubs_always_before_branch = group_size < 0;
  if (group_size < 0)
    stub_group_size = -group_size;
  else
    stub_group_size = group_size;
  if (stub_group_size == 1)
    {
      /* Default values.  */
      /* FIXME: not sure what these values should be */
      if (stubs_always_before_branch)
	{
	  stub_group_size = (1 << BRANCH_BITS);
	}
      else
	{
	  stub_group_size = (1 << BRANCH_BITS);
	}
    }

  group_sections (htab, stub_group_size, stubs_always_before_branch);

  switch (get_local_syms (output_bfd, info->input_bfds, info))
    {
    default:
      if (htab->all_local_syms)
	goto error_ret_free_local;
      return false;

    case 0:
      stub_changed = false;
      break;

    case 1:
      stub_changed = true;
      break;
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
		  enum elf_metag_stub_type stub_type;
		  struct elf_metag_stub_hash_entry *hsh;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct elf_metag_link_hash_entry *hh;
		  char *stub_name;
		  const asection *id_sec;

		  r_type = ELF32_R_TYPE (irela->r_info);
		  r_indx = ELF32_R_SYM (irela->r_info);

		  if (r_type >= (unsigned int) R_METAG_MAX)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }

		  /* Only look for stubs on CALLR and B instructions.  */
		  if (!(r_type == (unsigned int) R_METAG_RELBRANCH ||
			r_type == (unsigned int) R_METAG_RELBRANCH_PLT))
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
		      hh = ((struct elf_metag_link_hash_entry *)
			    elf_sym_hashes (input_bfd)[e_indx]);

		      while (hh->eh.root.type == bfd_link_hash_indirect
			     || hh->eh.root.type == bfd_link_hash_warning)
			hh = ((struct elf_metag_link_hash_entry *)
			      hh->eh.root.u.i.link);

		      if (hh->eh.root.type == bfd_link_hash_defined
			  || hh->eh.root.type == bfd_link_hash_defweak)
			{
			  sym_sec = hh->eh.root.u.def.section;
			  sym_value = hh->eh.root.u.def.value;
			  if (hh->eh.plt.offset != (bfd_vma) -1
			      && hh->eh.dynindx != -1
			      && r_type == (unsigned int) R_METAG_RELBRANCH_PLT)
			    {
			      sym_sec = htab->etab.splt;
			      sym_value = hh->eh.plt.offset;
			    }

			  if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			  else
			    continue;
			}
		      else if (hh->eh.root.type == bfd_link_hash_undefweak)
			{
			  if (! bfd_link_pic (info))
			    continue;
			}
		      else if (hh->eh.root.type == bfd_link_hash_undefined)
			{
			  if (! (info->unresolved_syms_in_objects == RM_IGNORE
				 && (ELF_ST_VISIBILITY (hh->eh.other)
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
		  stub_type = metag_type_of_stub (section, irela, hh,
						  destination, info);
		  if (stub_type == metag_stub_none)
		    continue;

		  /* Support for grouping stub sections.  */
		  id_sec = htab->stub_group[section->id].link_sec;

		  /* Get the name of this stub.  */
		  stub_name = metag_stub_name (id_sec, sym_sec, hh, irela);
		  if (!stub_name)
		    goto error_ret_free_internal;

		  hsh = metag_stub_hash_lookup (&htab->bstab,
						stub_name,
						false, false);
		  if (hsh != NULL)
		    {
		      /* The proper stub has already been created.  */
		      free (stub_name);
		      continue;
		    }

		  hsh = metag_add_stub (stub_name, section, htab);
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

      bfd_hash_traverse (&htab->bstab, metag_size_one_stub, htab);

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
   table.  This function is called via metagelf_finish in the linker.  */

bool
elf_metag_build_stubs (struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct elf_metag_link_hash_table *htab;

  htab = metag_link_hash_table (info);

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
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
  bfd_hash_traverse (table, metag_build_one_stub, info);

  return true;
}

/* Return TRUE if SYM represents a local label symbol.  */

static bool
elf_metag_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  if (name[0] == '$' && name[1] == 'L')
    return 1;
  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elf_metag_plt_sym_val (bfd_vma i, const asection *plt,
		       const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * PLT_ENTRY_SIZE;
}

#define ELF_ARCH		bfd_arch_metag
#define ELF_TARGET_ID		METAG_ELF_DATA
#define ELF_MACHINE_CODE	EM_METAG
#define ELF_MAXPAGESIZE		0x4000
#define ELF_COMMONPAGESIZE	0x1000

#define TARGET_LITTLE_SYM	metag_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-metag"

#define elf_symbol_leading_char '_'

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			metag_info_to_howto_rela

#define bfd_elf32_bfd_is_local_label_name	elf_metag_is_local_label_name
#define bfd_elf32_bfd_link_hash_table_create \
	elf_metag_link_hash_table_create
#define elf_backend_relocate_section		elf_metag_relocate_section
#define elf_backend_gc_mark_hook		elf_metag_gc_mark_hook
#define elf_backend_check_relocs		elf_metag_check_relocs
#define elf_backend_create_dynamic_sections	elf_metag_create_dynamic_sections
#define elf_backend_adjust_dynamic_symbol	elf_metag_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol	elf_metag_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections	elf_metag_finish_dynamic_sections
#define elf_backend_size_dynamic_sections	elf_metag_size_dynamic_sections
#define elf_backend_omit_section_dynsym \
	_bfd_elf_omit_section_dynsym_all
#define elf_backend_init_file_header		elf_metag_init_file_header
#define elf_backend_reloc_type_class		elf_metag_reloc_type_class
#define elf_backend_copy_indirect_symbol	elf_metag_copy_indirect_symbol
#define elf_backend_plt_sym_val		elf_metag_plt_sym_val

#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_rela_normal			1
#define elf_backend_want_got_plt		1
#define elf_backend_want_got_sym		0
#define elf_backend_want_plt_sym		0
#define elf_backend_plt_readonly		1
#define elf_backend_dtrel_excludes_plt		1
#define elf_backend_want_dynrelro		1

#define bfd_elf32_bfd_reloc_type_lookup	metag_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	metag_reloc_name_lookup

#include "elf32-target.h"
