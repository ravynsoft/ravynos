/* Copyright (C) 2006-2023 Free Software Foundation, Inc.

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

/* No relocation.  */
  HOWTO (R_SH_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit absolute relocation.  Setting partial_inplace to TRUE and
     src_mask to a non-zero value is similar to the COFF toolchain.  */
  HOWTO (R_SH_DIR32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 SH_ELF_RELOC,		/* special_function */
	 "R_SH_DIR32",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit PC relative relocation.  */
  HOWTO (R_SH_REL32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_REL32",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit PC relative branch divided by 2.  */
  HOWTO (R_SH_DIR8WPN,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8WPN",	/* name */
	 true,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 12 bit PC relative branch divided by 2.  */
  /* This cannot be partial_inplace because relaxation can't know the
     eventual value of a symbol.  */
  HOWTO (R_SH_IND12W,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_SH_IND12W",		/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xfff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit unsigned PC relative divided by 4.  */
  HOWTO (R_SH_DIR8WPL,		/* type */
	 2,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8WPL",	/* name */
	 true,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit unsigned PC relative divided by 2.  */
  HOWTO (R_SH_DIR8WPZ,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8WPZ",	/* name */
	 true,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit GBR relative.  FIXME: This only makes sense if we have some
     special symbol for the GBR relative area, and that is not
     implemented.  */
  HOWTO (R_SH_DIR8BP,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8BP",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit GBR relative divided by 2.  FIXME: This only makes sense if
     we have some special symbol for the GBR relative area, and that
     is not implemented.  */
  HOWTO (R_SH_DIR8W,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8W",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit GBR relative divided by 4.  FIXME: This only makes sense if
     we have some special symbol for the GBR relative area, and that
     is not implemented.  */
  HOWTO (R_SH_DIR8L,		/* type */
	 2,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DIR8L",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit PC relative divided by 2 - but specified in a very odd way.  */
  HOWTO (R_SH_LOOP_START,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_LOOP_START",	/* name */
	 true,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* 8 bit PC relative divided by 2 - but specified in a very odd way.  */
  HOWTO (R_SH_LOOP_END,		/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_LOOP_END",	/* name */
	 true,			/* partial_inplace */
	 0xff,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

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

  /* The remaining relocs are a GNU extension used for relaxing.  The
     final pass of the linker never needs to do anything with any of
     these relocs.  Any required operations are handled by the
     relaxation code.  */

  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_SH_GNU_VTINHERIT, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_SH_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_SH_GNU_VTENTRY,     /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,	/* special_function */
	 "R_SH_GNU_VTENTRY",   /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* An 8 bit switch table entry.  This is generated for an expression
     such as ``.word L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_SH_SWITCH8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_SWITCH8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit switch table entry.  This is generated for an expression
     such as ``.word L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_SH_SWITCH16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_SWITCH16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 32 bit switch table entry.  This is generated for an expression
     such as ``.long L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_SH_SWITCH32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_SWITCH32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* Indicates a .uses pseudo-op.  The compiler will generate .uses
     pseudo-ops when it finds a function call which can be relaxed.
     The offset field holds the PC relative offset to the instruction
     which loads the register used in the function call.  */
  HOWTO (R_SH_USES,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_USES",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* The assembler will generate this reloc for addresses referred to
     by the register loads associated with USES relocs.  The offset
     field holds the number of times the address is referenced in the
     object file.  */
  HOWTO (R_SH_COUNT,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_COUNT",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* Indicates an alignment statement.  The offset field is the power
     of 2 to which subsequent portions of the object file must be
     aligned.  */
  HOWTO (R_SH_ALIGN,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_ALIGN",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* The assembler will generate this reloc before a block of
     instructions.  A section should be processed as assuming it
     contains data, unless this reloc is seen.  */
  HOWTO (R_SH_CODE,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_CODE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* The assembler will generate this reloc after a block of
     instructions when it sees data that is not instructions.  */
  HOWTO (R_SH_DATA,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_DATA",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* The assembler generates this reloc for each label within a block
     of instructions.  This permits the linker to avoid swapping
     instructions which are the targets of branches.  */
  HOWTO (R_SH_LABEL,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 sh_elf_ignore_reloc,	/* special_function */
	 "R_SH_LABEL",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* The next 12 are only supported via linking in SHC-generated objects.  */
  HOWTO (R_SH_DIR16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8UL,		/* type */
	 2,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8UL",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8UW,		/* type */
	 1,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8UW",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8U,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8U",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8SW,		/* type */
	 1,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8SW",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR8S,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR8S",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR4UL,		/* type */
	 2,			/* rightshift */
	 1,			/* size */
	 4,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR4UL",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0f,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR4UW,		/* type */
	 1,			/* rightshift */
	 1,			/* size */
	 4,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR4UW",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0f,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_DIR4U,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 4,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR4U",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0f,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_PSHA,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 7,			/* bitsize */
	 false,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_PSHA",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0f,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_PSHL,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 7,			/* bitsize */
	 false,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_PSHL",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0f,			/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (45),
  EMPTY_HOWTO (46),
  EMPTY_HOWTO (47),
  EMPTY_HOWTO (48),
  EMPTY_HOWTO (49),
  EMPTY_HOWTO (50),
  EMPTY_HOWTO (51),

  EMPTY_HOWTO (52),

  HOWTO (R_SH_DIR16S,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_DIR16S",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (54),
  EMPTY_HOWTO (55),
  EMPTY_HOWTO (56),
  EMPTY_HOWTO (57),
  EMPTY_HOWTO (58),
  EMPTY_HOWTO (59),
  EMPTY_HOWTO (60),
  EMPTY_HOWTO (61),
  EMPTY_HOWTO (62),
  EMPTY_HOWTO (63),
  EMPTY_HOWTO (64),
  EMPTY_HOWTO (65),
  EMPTY_HOWTO (66),
  EMPTY_HOWTO (67),
  EMPTY_HOWTO (68),
  EMPTY_HOWTO (69),
  EMPTY_HOWTO (70),
  EMPTY_HOWTO (71),
  EMPTY_HOWTO (72),
  EMPTY_HOWTO (73),
  EMPTY_HOWTO (74),
  EMPTY_HOWTO (75),
  EMPTY_HOWTO (76),
  EMPTY_HOWTO (77),
  EMPTY_HOWTO (78),
  EMPTY_HOWTO (79),
  EMPTY_HOWTO (80),
  EMPTY_HOWTO (81),
  EMPTY_HOWTO (82),
  EMPTY_HOWTO (83),
  EMPTY_HOWTO (84),
  EMPTY_HOWTO (85),
  EMPTY_HOWTO (86),
  EMPTY_HOWTO (87),
  EMPTY_HOWTO (88),
  EMPTY_HOWTO (89),
  EMPTY_HOWTO (90),
  EMPTY_HOWTO (91),
  EMPTY_HOWTO (92),
  EMPTY_HOWTO (93),
  EMPTY_HOWTO (94),
  EMPTY_HOWTO (95),
  EMPTY_HOWTO (96),
  EMPTY_HOWTO (97),
  EMPTY_HOWTO (98),
  EMPTY_HOWTO (99),
  EMPTY_HOWTO (100),
  EMPTY_HOWTO (101),
  EMPTY_HOWTO (102),
  EMPTY_HOWTO (103),
  EMPTY_HOWTO (104),
  EMPTY_HOWTO (105),
  EMPTY_HOWTO (106),
  EMPTY_HOWTO (107),
  EMPTY_HOWTO (108),
  EMPTY_HOWTO (109),
  EMPTY_HOWTO (110),
  EMPTY_HOWTO (111),
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
  EMPTY_HOWTO (128),
  EMPTY_HOWTO (129),
  EMPTY_HOWTO (130),
  EMPTY_HOWTO (131),
  EMPTY_HOWTO (132),
  EMPTY_HOWTO (133),
  EMPTY_HOWTO (134),
  EMPTY_HOWTO (135),
  EMPTY_HOWTO (136),
  EMPTY_HOWTO (137),
  EMPTY_HOWTO (138),
  EMPTY_HOWTO (139),
  EMPTY_HOWTO (140),
  EMPTY_HOWTO (141),
  EMPTY_HOWTO (142),
  EMPTY_HOWTO (143),

  HOWTO (R_SH_TLS_GD_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_GD_32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_LD_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_LD_32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_LDO_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_LDO_32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_IE_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_IE_32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_LE_32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_LE_32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_DTPMOD32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_DTPOFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_DTPOFF32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_TLS_TPOFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* */
	 "R_SH_TLS_TPOFF32",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (152),
  EMPTY_HOWTO (153),
  EMPTY_HOWTO (154),
  EMPTY_HOWTO (155),
  EMPTY_HOWTO (156),
  EMPTY_HOWTO (157),
  EMPTY_HOWTO (158),
  EMPTY_HOWTO (159),

  HOWTO (R_SH_GOT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_GOT32",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_PLT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_PLT32",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_SH_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_COPY",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_GLOB_DAT,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_GLOB_DAT",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_JMP_SLOT,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_JMP_SLOT",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_RELATIVE,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_RELATIVE",	/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_GOTOFF,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_GOTOFF",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_SH_GOTPC,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_GOTPC",		/* name */
	 SH_PARTIAL32,		/* partial_inplace */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_SH_GOTPLT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* */
	 "R_SH_GOTPLT32",	/* name */
	 false,			/* partial_inplace */
	 /* ??? Why not 0?  */
	 SH_SRC_MASK32,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (169),
  EMPTY_HOWTO (170),
  EMPTY_HOWTO (171),
  EMPTY_HOWTO (172),
  EMPTY_HOWTO (173),
  EMPTY_HOWTO (174),
  EMPTY_HOWTO (175),
  EMPTY_HOWTO (176),
  EMPTY_HOWTO (177),
  EMPTY_HOWTO (178),
  EMPTY_HOWTO (179),
  EMPTY_HOWTO (180),
  EMPTY_HOWTO (181),
  EMPTY_HOWTO (182),
  EMPTY_HOWTO (183),
  EMPTY_HOWTO (184),
  EMPTY_HOWTO (185),
  EMPTY_HOWTO (186),
  EMPTY_HOWTO (187),
  EMPTY_HOWTO (188),
  EMPTY_HOWTO (189),
  EMPTY_HOWTO (190),
  EMPTY_HOWTO (191),
  EMPTY_HOWTO (192),
  EMPTY_HOWTO (193),
  EMPTY_HOWTO (194),
  EMPTY_HOWTO (195),
  EMPTY_HOWTO (196),
  EMPTY_HOWTO (197),
  EMPTY_HOWTO (198),
  EMPTY_HOWTO (199),
  EMPTY_HOWTO (200),

  /* FDPIC-relative offset to a GOT entry, for movi20.  */
  HOWTO (R_SH_GOT20,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 20,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOT20",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00f0ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* FDPIC-relative offset to a data object, for movi20.  */
  HOWTO (R_SH_GOTOFF20,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 20,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOTOFF20",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00f0ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* FDPIC-relative offset to a GOT entry for a function descriptor.  */
  HOWTO (R_SH_GOTFUNCDESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOTFUNCDESC",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* FDPIC-relative offset to a GOT entry for a function descriptor,
     for movi20.  */
  HOWTO (R_SH_GOTFUNCDESC20,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 20,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOTFUNCDESC20",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00f0ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* FDPIC-relative offset to a function descriptor.  */
  HOWTO (R_SH_GOTOFFFUNCDESC,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOTOFFFUNCDESC",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* FDPIC-relative offset to a function descriptor, for movi20.  */
  HOWTO (R_SH_GOTOFFFUNCDESC20,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 20,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_GOTOFFFUNCDESC20", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00f0ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Address of an official function descriptor.  */
  HOWTO (R_SH_FUNCDESC,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_FUNCDESC",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Function descriptor to be filled in by the dynamic linker.  */
  HOWTO (R_SH_FUNCDESC_VALUE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_SH_FUNCDESC_VALUE", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

#undef SH_PARTIAL32
#undef SH_SRC_MASK32
#undef SH_ELF_RELOC
