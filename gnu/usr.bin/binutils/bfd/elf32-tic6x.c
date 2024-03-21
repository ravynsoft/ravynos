/* 32-bit ELF support for TI C6X
   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Contributed by Joseph Myers <joseph@codesourcery.com>
		  Bernd Schmidt  <bernds@codesourcery.com>

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
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/tic6x.h"
#include "elf32-tic6x.h"

#define ELF_DYNAMIC_INTERPRETER "/lib/ld-uClibc.so.0"

/* DSBT binaries have a default 128K stack.  */
#define DEFAULT_STACK_SIZE 0x20000

/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE 24

/* TI C6X ELF linker hash table.  */

struct elf32_tic6x_link_hash_table
{
  struct elf_link_hash_table elf;

  /* C6X specific command line arguments.  */
  struct elf32_tic6x_params params;

  /* The output BFD, for convenience.  */
  bfd *obfd;

  /* The .dsbt section.  */
  asection *dsbt;
};

/* Get the TI C6X ELF linker hash table from a link_info structure.  */

#define elf32_tic6x_hash_table(p) \
  ((struct elf32_tic6x_link_hash_table *) ((p)->hash))

typedef enum
{
  DELETE_EXIDX_ENTRY,
  INSERT_EXIDX_CANTUNWIND_AT_END
}
tic6x_unwind_edit_type;

/* A (sorted) list of edits to apply to an unwind table.  */
typedef struct tic6x_unwind_table_edit
{
  tic6x_unwind_edit_type type;
  /* Note: we sometimes want to insert an unwind entry corresponding to a
     section different from the one we're currently writing out, so record the
     (text) section this edit relates to here.  */
  asection *linked_section;
  unsigned int index;
  struct tic6x_unwind_table_edit *next;
}
tic6x_unwind_table_edit;

typedef struct _tic6x_elf_section_data
{
  /* Information about mapping symbols.  */
  struct bfd_elf_section_data elf;
  /* Information about unwind tables.  */
  union
  {
    /* Unwind info attached to a text section.  */
    struct
    {
      asection *tic6x_exidx_sec;
    } text;

    /* Unwind info attached to an .c6xabi.exidx section.  */
    struct
    {
      tic6x_unwind_table_edit *unwind_edit_list;
      tic6x_unwind_table_edit *unwind_edit_tail;
    } exidx;
  } u;
}
_tic6x_elf_section_data;

#define elf32_tic6x_section_data(sec) \
  ((_tic6x_elf_section_data *) elf_section_data (sec))

struct elf32_tic6x_obj_tdata
{
  struct elf_obj_tdata root;

  /* Whether to use RELA relocations when generating relocations.
     This is a per-object flag to allow the assembler to generate REL
     relocations for use in linker testcases.  */
  bool use_rela_p;
};

#define elf32_tic6x_tdata(abfd) \
  ((struct elf32_tic6x_obj_tdata *) (abfd)->tdata.any)

#define is_tic6x_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_tdata (bfd) != NULL \
   && elf_object_id (bfd) == TIC6X_ELF_DATA)

/* C6X ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the gp pointer, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection tic6x_elf_scom_section;
static const asymbol tic6x_elf_scom_symbol =
  GLOBAL_SYM_INIT (".scommon", &tic6x_elf_scom_section);
static asection tic6x_elf_scom_section =
  BFD_FAKE_SECTION (tic6x_elf_scom_section, &tic6x_elf_scom_symbol,
		    ".scommon", 0, SEC_IS_COMMON | SEC_SMALL_DATA);

static reloc_howto_type elf32_tic6x_howto_table[] =
{
  HOWTO (R_C6000_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_PCR_S21,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 true,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S21",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0fffff80,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S12,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S12",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0fff0000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S10,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 13,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S10",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fe000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S7,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 7,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S7",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007f0000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_ABS_S16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS_S16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS_L16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS_L16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS_H16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS_H16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_U15_B,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_B",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_U15_H,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_H",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_U15_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_W",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_S16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_S16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_B,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_B",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_H,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_H",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_W",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_H16_B,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_H16_B",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_H16_H,	/* type */
	 17,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_H16_H",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_H16_W,	/* type */
	 18,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_H16_W",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_GOT_U15_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_GOT_U15_W",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_GOT_L16_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_GOT_L16_W",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_GOT_H16_W,	/* type */
	 18,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_GOT_H16_W",/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_DSBT_INDEX,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_DSBT_INDEX",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_PREL31,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 31,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PREL31",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x7fffffff,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_COPY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_JUMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_JUMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_EHTYPE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_EHTYPE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_PCR_H16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_H16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_L16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_L16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff80,		/* dst_mask */
	 true),			/* pcrel_offset */
  EMPTY_HOWTO (31),
  EMPTY_HOWTO (32),
  EMPTY_HOWTO (33),
  EMPTY_HOWTO (34),
  EMPTY_HOWTO (35),
  EMPTY_HOWTO (36),
  EMPTY_HOWTO (37),
  EMPTY_HOWTO (38),
  EMPTY_HOWTO (39),
  EMPTY_HOWTO (40),
  EMPTY_HOWTO (41),
  EMPTY_HOWTO (42),
  EMPTY_HOWTO (43),
  EMPTY_HOWTO (44),
  EMPTY_HOWTO (45),
  EMPTY_HOWTO (46),
  EMPTY_HOWTO (47),
  EMPTY_HOWTO (48),
  EMPTY_HOWTO (49),
  EMPTY_HOWTO (50),
  EMPTY_HOWTO (51),
  EMPTY_HOWTO (52),
  EMPTY_HOWTO (53),
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
  EMPTY_HOWTO (144),
  EMPTY_HOWTO (145),
  EMPTY_HOWTO (146),
  EMPTY_HOWTO (147),
  EMPTY_HOWTO (148),
  EMPTY_HOWTO (149),
  EMPTY_HOWTO (150),
  EMPTY_HOWTO (151),
  EMPTY_HOWTO (152),
  EMPTY_HOWTO (153),
  EMPTY_HOWTO (154),
  EMPTY_HOWTO (155),
  EMPTY_HOWTO (156),
  EMPTY_HOWTO (157),
  EMPTY_HOWTO (158),
  EMPTY_HOWTO (159),
  EMPTY_HOWTO (160),
  EMPTY_HOWTO (161),
  EMPTY_HOWTO (162),
  EMPTY_HOWTO (163),
  EMPTY_HOWTO (164),
  EMPTY_HOWTO (165),
  EMPTY_HOWTO (166),
  EMPTY_HOWTO (167),
  EMPTY_HOWTO (168),
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
  EMPTY_HOWTO (201),
  EMPTY_HOWTO (202),
  EMPTY_HOWTO (203),
  EMPTY_HOWTO (204),
  EMPTY_HOWTO (205),
  EMPTY_HOWTO (206),
  EMPTY_HOWTO (207),
  EMPTY_HOWTO (208),
  EMPTY_HOWTO (209),
  EMPTY_HOWTO (210),
  EMPTY_HOWTO (211),
  EMPTY_HOWTO (212),
  EMPTY_HOWTO (213),
  EMPTY_HOWTO (214),
  EMPTY_HOWTO (215),
  EMPTY_HOWTO (216),
  EMPTY_HOWTO (217),
  EMPTY_HOWTO (218),
  EMPTY_HOWTO (219),
  EMPTY_HOWTO (220),
  EMPTY_HOWTO (221),
  EMPTY_HOWTO (222),
  EMPTY_HOWTO (223),
  EMPTY_HOWTO (224),
  EMPTY_HOWTO (225),
  EMPTY_HOWTO (226),
  EMPTY_HOWTO (227),
  EMPTY_HOWTO (228),
  EMPTY_HOWTO (229),
  EMPTY_HOWTO (230),
  EMPTY_HOWTO (231),
  EMPTY_HOWTO (232),
  EMPTY_HOWTO (233),
  EMPTY_HOWTO (234),
  EMPTY_HOWTO (235),
  EMPTY_HOWTO (236),
  EMPTY_HOWTO (237),
  EMPTY_HOWTO (238),
  EMPTY_HOWTO (239),
  EMPTY_HOWTO (240),
  EMPTY_HOWTO (241),
  EMPTY_HOWTO (242),
  EMPTY_HOWTO (243),
  EMPTY_HOWTO (244),
  EMPTY_HOWTO (245),
  EMPTY_HOWTO (246),
  EMPTY_HOWTO (247),
  EMPTY_HOWTO (248),
  EMPTY_HOWTO (249),
  EMPTY_HOWTO (250),
  EMPTY_HOWTO (251),
  EMPTY_HOWTO (252),
  HOWTO (R_C6000_ALIGN,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ALIGN",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_FPHEAD,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_FPHEAD",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_NOCMP,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_NOCMP",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false)			/* pcrel_offset */
};

static reloc_howto_type elf32_tic6x_howto_table_rel[] =
{
  HOWTO (R_C6000_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_NONE",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS8",	/* name */
	 true,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_PCR_S21,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 true,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S21",	/* name */
	 true,			/* partial_inplace */
	 0x0fffff80,		/* src_mask */
	 0x0fffff80,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S12,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 12,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S12",	/* name */
	 true,			/* partial_inplace */
	 0x0fff0000,		/* src_mask */
	 0x0fff0000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S10,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 13,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S10",	/* name */
	 true,			/* partial_inplace */
	 0x007fe000,		/* src_mask */
	 0x007fe000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_PCR_S7,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 7,			/* bitsize */
	 true,			/* pc_relative */
	 16,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PCR_S7",	/* name */
	 true,			/* partial_inplace */
	 0x007f0000,		/* src_mask */
	 0x007f0000,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_ABS_S16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS_S16",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_ABS_L16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ABS_L16",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  EMPTY_HOWTO (R_C6000_ABS_H16),
  HOWTO (R_C6000_SBR_U15_B,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_B",	/* name */
	 true,			/* partial_inplace */
	 0x007fff00,		/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_U15_H,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_H",	/* name */
	 true,			/* partial_inplace */
	 0x007fff00,		/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_U15_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_U15_W",	/* name */
	 true,			/* partial_inplace */
	 0x007fff00,		/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_S16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_signed,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_S16",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_B,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_B",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_H,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_H",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_L16_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_L16_W",	/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  EMPTY_HOWTO (R_C6000_SBR_H16_B),
  EMPTY_HOWTO (R_C6000_SBR_H16_H),
  EMPTY_HOWTO (R_C6000_SBR_H16_W),
  HOWTO (R_C6000_SBR_GOT_U15_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_GOT_U15_W",/* name */
	 true,			/* partial_inplace */
	 0x007fff00,		/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_SBR_GOT_L16_W,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_SBR_GOT_L16_W",/* name */
	 true,			/* partial_inplace */
	 0x007fff80,		/* src_mask */
	 0x007fff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  EMPTY_HOWTO (R_C6000_SBR_GOT_H16_W),
  HOWTO (R_C6000_DSBT_INDEX,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_DSBT_INDEX",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0x007fff00,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_PREL31,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 31,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_PREL31",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0x7fffffff,		/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_C6000_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_COPY",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_JUMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_JUMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_EHTYPE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_EHTYPE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  EMPTY_HOWTO (R_C6000_PCR_H16),
  EMPTY_HOWTO (R_C6000_PCR_L16),
  EMPTY_HOWTO (31),
  EMPTY_HOWTO (32),
  EMPTY_HOWTO (33),
  EMPTY_HOWTO (34),
  EMPTY_HOWTO (35),
  EMPTY_HOWTO (36),
  EMPTY_HOWTO (37),
  EMPTY_HOWTO (38),
  EMPTY_HOWTO (39),
  EMPTY_HOWTO (40),
  EMPTY_HOWTO (41),
  EMPTY_HOWTO (42),
  EMPTY_HOWTO (43),
  EMPTY_HOWTO (44),
  EMPTY_HOWTO (45),
  EMPTY_HOWTO (46),
  EMPTY_HOWTO (47),
  EMPTY_HOWTO (48),
  EMPTY_HOWTO (49),
  EMPTY_HOWTO (50),
  EMPTY_HOWTO (51),
  EMPTY_HOWTO (52),
  EMPTY_HOWTO (53),
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
  EMPTY_HOWTO (144),
  EMPTY_HOWTO (145),
  EMPTY_HOWTO (146),
  EMPTY_HOWTO (147),
  EMPTY_HOWTO (148),
  EMPTY_HOWTO (149),
  EMPTY_HOWTO (150),
  EMPTY_HOWTO (151),
  EMPTY_HOWTO (152),
  EMPTY_HOWTO (153),
  EMPTY_HOWTO (154),
  EMPTY_HOWTO (155),
  EMPTY_HOWTO (156),
  EMPTY_HOWTO (157),
  EMPTY_HOWTO (158),
  EMPTY_HOWTO (159),
  EMPTY_HOWTO (160),
  EMPTY_HOWTO (161),
  EMPTY_HOWTO (162),
  EMPTY_HOWTO (163),
  EMPTY_HOWTO (164),
  EMPTY_HOWTO (165),
  EMPTY_HOWTO (166),
  EMPTY_HOWTO (167),
  EMPTY_HOWTO (168),
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
  EMPTY_HOWTO (201),
  EMPTY_HOWTO (202),
  EMPTY_HOWTO (203),
  EMPTY_HOWTO (204),
  EMPTY_HOWTO (205),
  EMPTY_HOWTO (206),
  EMPTY_HOWTO (207),
  EMPTY_HOWTO (208),
  EMPTY_HOWTO (209),
  EMPTY_HOWTO (210),
  EMPTY_HOWTO (211),
  EMPTY_HOWTO (212),
  EMPTY_HOWTO (213),
  EMPTY_HOWTO (214),
  EMPTY_HOWTO (215),
  EMPTY_HOWTO (216),
  EMPTY_HOWTO (217),
  EMPTY_HOWTO (218),
  EMPTY_HOWTO (219),
  EMPTY_HOWTO (220),
  EMPTY_HOWTO (221),
  EMPTY_HOWTO (222),
  EMPTY_HOWTO (223),
  EMPTY_HOWTO (224),
  EMPTY_HOWTO (225),
  EMPTY_HOWTO (226),
  EMPTY_HOWTO (227),
  EMPTY_HOWTO (228),
  EMPTY_HOWTO (229),
  EMPTY_HOWTO (230),
  EMPTY_HOWTO (231),
  EMPTY_HOWTO (232),
  EMPTY_HOWTO (233),
  EMPTY_HOWTO (234),
  EMPTY_HOWTO (235),
  EMPTY_HOWTO (236),
  EMPTY_HOWTO (237),
  EMPTY_HOWTO (238),
  EMPTY_HOWTO (239),
  EMPTY_HOWTO (240),
  EMPTY_HOWTO (241),
  EMPTY_HOWTO (242),
  EMPTY_HOWTO (243),
  EMPTY_HOWTO (244),
  EMPTY_HOWTO (245),
  EMPTY_HOWTO (246),
  EMPTY_HOWTO (247),
  EMPTY_HOWTO (248),
  EMPTY_HOWTO (249),
  EMPTY_HOWTO (250),
  EMPTY_HOWTO (251),
  EMPTY_HOWTO (252),
  HOWTO (R_C6000_ALIGN,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_ALIGN",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_FPHEAD,	/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_FPHEAD",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_C6000_NOCMP,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_C6000_NOCMP",	/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false)			/* pcrel_offset */
};

/* Map BFD relocations to ELF relocations.  */

typedef struct
{
  bfd_reloc_code_real_type bfd_reloc_val;
  enum elf_tic6x_reloc_type elf_reloc_val;
} tic6x_reloc_map;

static const tic6x_reloc_map elf32_tic6x_reloc_map[] =
  {
    { BFD_RELOC_NONE, R_C6000_NONE },
    { BFD_RELOC_32, R_C6000_ABS32 },
    { BFD_RELOC_16, R_C6000_ABS16 },
    { BFD_RELOC_8, R_C6000_ABS8 },
    { BFD_RELOC_C6000_PCR_S21, R_C6000_PCR_S21 },
    { BFD_RELOC_C6000_PCR_S12, R_C6000_PCR_S12 },
    { BFD_RELOC_C6000_PCR_S10, R_C6000_PCR_S10 },
    { BFD_RELOC_C6000_PCR_S7, R_C6000_PCR_S7 },
    { BFD_RELOC_C6000_ABS_S16, R_C6000_ABS_S16 },
    { BFD_RELOC_C6000_ABS_L16, R_C6000_ABS_L16 },
    { BFD_RELOC_C6000_ABS_H16, R_C6000_ABS_H16 },
    { BFD_RELOC_C6000_SBR_U15_B, R_C6000_SBR_U15_B },
    { BFD_RELOC_C6000_SBR_U15_H, R_C6000_SBR_U15_H },
    { BFD_RELOC_C6000_SBR_U15_W, R_C6000_SBR_U15_W },
    { BFD_RELOC_C6000_SBR_S16, R_C6000_SBR_S16 },
    { BFD_RELOC_C6000_SBR_L16_B, R_C6000_SBR_L16_B },
    { BFD_RELOC_C6000_SBR_L16_H, R_C6000_SBR_L16_H },
    { BFD_RELOC_C6000_SBR_L16_W, R_C6000_SBR_L16_W },
    { BFD_RELOC_C6000_SBR_H16_B, R_C6000_SBR_H16_B },
    { BFD_RELOC_C6000_SBR_H16_H, R_C6000_SBR_H16_H },
    { BFD_RELOC_C6000_SBR_H16_W, R_C6000_SBR_H16_W },
    { BFD_RELOC_C6000_SBR_GOT_U15_W, R_C6000_SBR_GOT_U15_W },
    { BFD_RELOC_C6000_SBR_GOT_L16_W, R_C6000_SBR_GOT_L16_W },
    { BFD_RELOC_C6000_SBR_GOT_H16_W, R_C6000_SBR_GOT_H16_W },
    { BFD_RELOC_C6000_DSBT_INDEX, R_C6000_DSBT_INDEX },
    { BFD_RELOC_C6000_PREL31, R_C6000_PREL31 },
    { BFD_RELOC_C6000_COPY, R_C6000_COPY },
    { BFD_RELOC_C6000_JUMP_SLOT, R_C6000_JUMP_SLOT },
    { BFD_RELOC_C6000_EHTYPE, R_C6000_EHTYPE },
    { BFD_RELOC_C6000_PCR_H16, R_C6000_PCR_H16 },
    { BFD_RELOC_C6000_PCR_L16, R_C6000_PCR_L16 },
    { BFD_RELOC_C6000_ALIGN, R_C6000_ALIGN },
    { BFD_RELOC_C6000_FPHEAD, R_C6000_FPHEAD },
    { BFD_RELOC_C6000_NOCMP, R_C6000_NOCMP }
  };

static reloc_howto_type *
elf32_tic6x_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf32_tic6x_reloc_map); i++)
    if (elf32_tic6x_reloc_map[i].bfd_reloc_val == code)
      {
	enum elf_tic6x_reloc_type elf_reloc_val;
	reloc_howto_type *howto;

	elf_reloc_val = elf32_tic6x_reloc_map[i].elf_reloc_val;
	if (elf32_tic6x_tdata (abfd)->use_rela_p)
	  howto = &elf32_tic6x_howto_table[elf_reloc_val];
	else
	  howto = &elf32_tic6x_howto_table_rel[elf_reloc_val];

	/* Some relocations are RELA-only; do not return them for
	   REL.  */
	if (howto->name == NULL)
	  howto = NULL;

	return howto;
      }

  return NULL;
}

static reloc_howto_type *
elf32_tic6x_reloc_name_lookup (bfd *abfd, const char *r_name)
{
  if (elf32_tic6x_tdata (abfd)->use_rela_p)
    {
      unsigned int i;

      for (i = 0; i < ARRAY_SIZE (elf32_tic6x_howto_table); i++)
	if (elf32_tic6x_howto_table[i].name != NULL
	    && strcasecmp (elf32_tic6x_howto_table[i].name, r_name) == 0)
	  return &elf32_tic6x_howto_table[i];
    }
  else
    {
      unsigned int i;

      for (i = 0; i < ARRAY_SIZE (elf32_tic6x_howto_table_rel); i++)
	if (elf32_tic6x_howto_table_rel[i].name != NULL
	    && strcasecmp (elf32_tic6x_howto_table_rel[i].name, r_name) == 0)
	  return &elf32_tic6x_howto_table_rel[i];
    }

  return NULL;
}

static bool
elf32_tic6x_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, arelent *bfd_reloc,
			   Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (elf_reloc->r_info);
  if (r_type >= ARRAY_SIZE (elf32_tic6x_howto_table))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  bfd_reloc->howto = &elf32_tic6x_howto_table[r_type];
  if (bfd_reloc->howto == NULL || bfd_reloc->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

static bool
elf32_tic6x_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED, arelent *bfd_reloc,
			       Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (elf_reloc->r_info);
  if (r_type >= ARRAY_SIZE (elf32_tic6x_howto_table_rel))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  bfd_reloc->howto = &elf32_tic6x_howto_table_rel[r_type];
  if (bfd_reloc->howto == NULL || bfd_reloc->howto->name == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

void
elf32_tic6x_set_use_rela_p (bfd *abfd, bool use_rela_p)
{
  elf32_tic6x_tdata (abfd)->use_rela_p = use_rela_p;
}

/* Create a C6X ELF linker hash table.  */

static struct bfd_link_hash_table *
elf32_tic6x_link_hash_table_create (bfd *abfd)
{
  struct elf32_tic6x_link_hash_table *ret;
  size_t amt = sizeof (struct elf32_tic6x_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd,
				      _bfd_elf_link_hash_newfunc,
				      sizeof (struct elf_link_hash_entry),
				      TIC6X_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->obfd = abfd;
  ret->elf.is_relocatable_executable = 1;

  return &ret->elf.root;
}

static bool
elf32_tic6x_final_link (bfd *abfd, struct bfd_link_info *info)
{
  if (bfd_link_pic (info))
    {
      obj_attribute *out_attr;
      out_attr = elf_known_obj_attributes_proc (abfd);
      if (out_attr[Tag_ABI_PIC].i == 0)
	{
	  _bfd_error_handler (_("warning: generating a shared library "
				"containing non-PIC code"));
	}
      if (out_attr[Tag_ABI_PID].i == 0)
	{
	  _bfd_error_handler (_("warning: generating a shared library "
				"containing non-PID code"));
	}
    }
  /* Invoke the regular ELF backend linker to do all the work.  */
  if (!bfd_elf_final_link (abfd, info))
    return false;

  return true;
}

/* Called to pass PARAMS to the backend.  We store them in the hash table
   associated with INFO.  */

void
elf32_tic6x_setup (struct bfd_link_info *info,
		   struct elf32_tic6x_params *params)
{
  struct elf32_tic6x_link_hash_table *htab = elf32_tic6x_hash_table (info);
  htab->params = *params;
}

/* Determine if we're dealing with a DSBT object.  */

static bool
elf32_tic6x_using_dsbt (bfd *abfd)
{
  return bfd_elf_get_obj_attr_int (abfd, OBJ_ATTR_PROC,
				   Tag_ABI_DSBT);
}

/* Create .plt, .rela.plt, .got, .got.plt, .rela.got and .dsbt
   sections in DYNOBJ, and set up shortcuts to them in our hash
   table.  */

static bool
elf32_tic6x_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_tic6x_link_hash_table *htab;
  flagword flags;

  htab = elf32_tic6x_hash_table (info);
  if (htab == NULL)
    return false;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return false;

  /* Create .dsbt  */
  flags = (SEC_ALLOC | SEC_LOAD
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED);
  htab->dsbt = bfd_make_section_anyway_with_flags (dynobj, ".dsbt",
						   flags);
  if (htab->dsbt == NULL
      || !bfd_set_section_alignment (htab->dsbt, 2)
      || !bfd_set_section_alignment (htab->elf.splt, 5))
    return false;

  return true;
}

static bool
elf32_tic6x_mkobject (bfd *abfd)
{
  bool ret;

  ret = bfd_elf_allocate_object (abfd, sizeof (struct elf32_tic6x_obj_tdata),
				 TIC6X_ELF_DATA);
  if (ret)
    elf32_tic6x_set_use_rela_p (abfd, true);
  return ret;
}

/* Install relocation RELA into section SRELA, incrementing its
   reloc_count.  */

static void
elf32_tic6x_install_rela (bfd *output_bfd, asection *srela,
			  Elf_Internal_Rela *rela)
{
  bfd_byte *loc;
  bfd_vma off = srela->reloc_count++ * sizeof (Elf32_External_Rela);
  loc = srela->contents + off;
  BFD_ASSERT (off < srela->size);
  bfd_elf32_swap_reloca_out (output_bfd, rela, loc);
}

/* Create a dynamic reloc against the GOT at offset OFFSET.  The contents
   of the GOT at this offset have been initialized with the relocation.  */

static void
elf32_tic6x_make_got_dynreloc (bfd *output_bfd,
			       struct elf32_tic6x_link_hash_table *htab,
			       asection *sym_sec, bfd_vma offset)
{
  asection *sgot = htab->elf.sgot;
  Elf_Internal_Rela outrel;
  int dynindx;

  outrel.r_offset = sgot->output_section->vma + sgot->output_offset + offset;
  outrel.r_addend = bfd_get_32 (output_bfd, sgot->contents + offset);
  if (sym_sec && sym_sec->output_section
      && ! bfd_is_abs_section (sym_sec->output_section)
      && ! bfd_is_und_section (sym_sec->output_section))
    {
      dynindx = elf_section_data (sym_sec->output_section)->dynindx;
      outrel.r_addend -= sym_sec->output_section->vma;
    }
  else
    {
      dynindx = 0;
    }
  outrel.r_info = ELF32_R_INFO (dynindx, R_C6000_ABS32);
  elf32_tic6x_install_rela (output_bfd, htab->elf.srelgot, &outrel);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf32_tic6x_finish_dynamic_symbol (bfd * output_bfd,
				   struct bfd_link_info *info,
				   struct elf_link_hash_entry *h,
				   Elf_Internal_Sym * sym)
{
  struct elf32_tic6x_link_hash_table *htab;

  htab = elf32_tic6x_hash_table (info);

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index;
      bfd_vma got_section_offset, got_dp_offset, rela_offset;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      asection *plt, *gotplt, *relplt;
      const struct elf_backend_data *bed;

      bed = get_elf_backend_data (output_bfd);

      BFD_ASSERT (htab->elf.splt != NULL);
      plt = htab->elf.splt;
      gotplt = htab->elf.sgotplt;
      relplt = htab->elf.srelplt;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */

      if ((h->dynindx == -1
	   && !((h->forced_local || bfd_link_executable (info))
		&& h->def_regular
		&& h->type == STT_GNU_IFUNC))
	  || plt == NULL
	  || gotplt == NULL
	  || relplt == NULL)
	abort ();

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.

	 Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first three are reserved.

	 For static executables, we don't reserve anything.  */

      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
      got_section_offset = plt_index + bed->got_header_size / 4;
      got_dp_offset = got_section_offset + htab->params.dsbt_size;
      rela_offset = plt_index * sizeof (Elf32_External_Rela);

      got_section_offset *= 4;

      /* Fill in the entry in the procedure linkage table.  */

      /* ldw .d2t2 *+B14($GOT(f)), b2 */
      bfd_put_32 (output_bfd, got_dp_offset << 8 | 0x0100006e,
		  plt->contents + h->plt.offset);
      /* mvk .s2 low(rela_offset), b0 */
      bfd_put_32 (output_bfd, (rela_offset & 0xffff) << 7 | 0x0000002a,
		  plt->contents + h->plt.offset + 4);
      /* mvkh .s2 high(rela_offset), b0 */
      bfd_put_32 (output_bfd, ((rela_offset >> 16) & 0xffff) << 7 | 0x0000006a,
		  plt->contents + h->plt.offset + 8);
      /* nop 2 */
      bfd_put_32 (output_bfd, 0x00002000,
		  plt->contents + h->plt.offset + 12);
      /* b .s2 b2 */
      bfd_put_32 (output_bfd, 0x00080362,
		  plt->contents + h->plt.offset + 16);
      /* nop 5 */
      bfd_put_32 (output_bfd, 0x00008000,
		  plt->contents + h->plt.offset + 20);

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (plt->output_section->vma + plt->output_offset),
		  gotplt->contents + got_section_offset);

      /* Fill in the entry in the .rel.plt section.  */
      rela.r_offset = (gotplt->output_section->vma
		       + gotplt->output_offset
		       + got_section_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_C6000_JUMP_SLOT);
      rela.r_addend = 0;
      loc = relplt->contents + rela_offset;
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  */
	  sym->st_shndx = SHN_UNDEF;
	  sym->st_value = 0;
	}
    }

  if (h->got.offset != (bfd_vma) -1)
    {
      asection *sgot;
      asection *srela;

      /* This symbol has an entry in the global offset table.
	 Set it up.  */

      sgot = htab->elf.sgot;
      srela = htab->elf.srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info)
	  && (SYMBOLIC_BIND (info, h)
	      || h->dynindx == -1 || h->forced_local) && h->def_regular)
	{
	  asection *s = h->root.u.def.section;
	  elf32_tic6x_make_got_dynreloc (output_bfd, htab, s,
			     h->got.offset & ~(bfd_vma) 1);
	}
      else
	{
	  Elf_Internal_Rela outrel;
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      sgot->contents + (h->got.offset & ~(bfd_vma) 1));
	  outrel.r_offset = (sgot->output_section->vma
			   + sgot->output_offset
			   + (h->got.offset & ~(bfd_vma) 1));
	  outrel.r_info = ELF32_R_INFO (h->dynindx, R_C6000_ABS32);
	  outrel.r_addend = 0;

	  elf32_tic6x_install_rela (output_bfd, srela, &outrel);
	}
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rel;
      asection *s;

      /* This symbol needs a copy reloc.  Set it up.  */

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || htab->elf.srelbss == NULL
	  || htab->elf.sreldynrelro == NULL)
	abort ();

      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_C6000_COPY);
      rel.r_addend = 0;
      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;

      elf32_tic6x_install_rela (output_bfd, s, &rel);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (h == elf_hash_table (info)->hdynamic
      || h == elf_hash_table (info)->hgot)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Unwinding tables are not referenced directly.  This pass marks them as
   required if the corresponding code section is marked.  */

static bool
elf32_tic6x_gc_mark_extra_sections (struct bfd_link_info *info,
				    elf_gc_mark_hook_fn gc_mark_hook)
{
  bfd *sub;
  Elf_Internal_Shdr **elf_shdrp;
  bool again;

  _bfd_elf_gc_mark_extra_sections (info, gc_mark_hook);

  /* Marking EH data may cause additional code sections to be marked,
     requiring multiple passes.  */
  again = true;
  while (again)
    {
      again = false;
      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	{
	  asection *o;

	  if (! is_tic6x_elf (sub))
	    continue;

	  elf_shdrp = elf_elfsections (sub);
	  for (o = sub->sections; o != NULL; o = o->next)
	    {
	      Elf_Internal_Shdr *hdr;

	      hdr = &elf_section_data (o)->this_hdr;
	      if (hdr->sh_type == SHT_C6000_UNWIND
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
	}
    }

  return true;
}

/* Return TRUE if this is an unwinding table index.  */

static bool
is_tic6x_elf_unwind_section_name (const char *name)
{
  return (startswith (name, ELF_STRING_C6000_unwind)
	  || startswith (name, ELF_STRING_C6000_unwind_once));
}


/* Set the type and flags for an unwinding index table.  We do this by
   the section name, which is a hack, but ought to work.  */

static bool
elf32_tic6x_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
			   Elf_Internal_Shdr *hdr, asection *sec)
{
  const char * name;

  name = bfd_section_name (sec);

  if (is_tic6x_elf_unwind_section_name (name))
    {
      hdr->sh_type = SHT_C6000_UNWIND;
      hdr->sh_flags |= SHF_LINK_ORDER;
    }

  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf32_tic6x_adjust_dynamic_symbol (struct bfd_link_info *info,
				   struct elf_link_hash_entry *h)
{
  struct elf32_tic6x_link_hash_table *htab;
  bfd *dynobj;
  asection *s, *srel;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic && h->ref_regular && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC32 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
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
      h->non_got_ref = def->non_got_ref;
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

  htab = elf32_tic6x_hash_table (info);
  if (htab == NULL)
    return false;

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  /* We must generate a R_C6000_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  */
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
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

static bool
elf32_tic6x_new_section_hook (bfd *abfd, asection *sec)
{
  bool ret;

  /* Allocate target specific section data.  */
  if (!sec->used_by_bfd)
    {
      _tic6x_elf_section_data *sdata;
      size_t amt = sizeof (*sdata);

      sdata = (_tic6x_elf_section_data *) bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  ret = _bfd_elf_new_section_hook (abfd, sec);
  sec->use_rela_p = elf32_tic6x_tdata (abfd)->use_rela_p;

  return ret;
}

/* Return true if relocation REL against section SEC is a REL rather
   than RELA relocation.  RELOCS is the first relocation in the
   section and ABFD is the bfd that contains SEC.  */

static bool
elf32_tic6x_rel_relocation_p (bfd *abfd, asection *sec,
			      const Elf_Internal_Rela *relocs,
			      const Elf_Internal_Rela *rel)
{
  Elf_Internal_Shdr *rel_hdr;
  const struct elf_backend_data *bed;

  /* To determine which flavor of relocation this is, we depend on the
     fact that the INPUT_SECTION's REL_HDR is read before RELA_HDR.  */
  rel_hdr = elf_section_data (sec)->rel.hdr;
  if (rel_hdr == NULL)
    return false;
  bed = get_elf_backend_data (abfd);
  return ((size_t) (rel - relocs)
	  < NUM_SHDR_ENTRIES (rel_hdr) * bed->s->int_rels_per_ext_rel);
}

/* We need dynamic symbols for every section, since segments can
   relocate independently.  */
static bool
elf32_tic6x_link_omit_section_dynsym (bfd *output_bfd ATTRIBUTE_UNUSED,
				      struct bfd_link_info *info ATTRIBUTE_UNUSED,
				      asection *p)
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

static int
elf32_tic6x_relocate_section (bfd *output_bfd,
			      struct bfd_link_info *info,
			      bfd *input_bfd,
			      asection *input_section,
			      bfd_byte *contents,
			      Elf_Internal_Rela *relocs,
			      Elf_Internal_Sym *local_syms,
			      asection **local_sections)
{
  struct elf32_tic6x_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bool ok = true;

  htab = elf32_tic6x_hash_table (info);
  symtab_hdr = & elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);

  relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      int r_type;
      unsigned long r_symndx;
      arelent bfd_reloc;
      reloc_howto_type *howto;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma off, off2, relocation;
      bool unresolved_reloc;
      bfd_reloc_status_type r;
      struct bfd_link_hash_entry *sbh;
      bool is_rel;
      bool res;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      is_rel = elf32_tic6x_rel_relocation_p (input_bfd, input_section,
					     relocs, rel);

      if (is_rel)
	res = elf32_tic6x_info_to_howto_rel (input_bfd, &bfd_reloc, rel);
      else
	res = elf32_tic6x_info_to_howto (input_bfd, &bfd_reloc, rel);

      if (!res || (howto = bfd_reloc.howto) == NULL)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

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
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	{
	  if (is_rel
	      && sym != NULL
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      rel->r_addend = 0;
	      relocation = sec->output_offset + sym->st_value;
	      r = _bfd_relocate_contents (howto, input_bfd, relocation,
					  contents + rel->r_offset);
	      goto done_reloc;
	    }
	  continue;
	}

      switch (r_type)
	{
	case R_C6000_NONE:
	case R_C6000_ALIGN:
	case R_C6000_FPHEAD:
	case R_C6000_NOCMP:
	  /* No action needed.  */
	  continue;

	case R_C6000_PCR_S21:
	  /* A branch to an undefined weak symbol is turned into a
	     "b .s2 B3" instruction if the existing insn is of the
	     form "b .s2 symbol".  */
	  if (h ? h->root.type == bfd_link_hash_undefweak
	      && (htab->elf.splt == NULL || h->plt.offset == (bfd_vma) -1)
	      : r_symndx != STN_UNDEF && bfd_is_und_section (sec))
	    {
	      unsigned long oldval;
	      oldval = bfd_get_32 (input_bfd, contents + rel->r_offset);

	      if ((oldval & 0x7e) == 0x12)
		{
		  oldval &= 0xF0000001;
		  bfd_put_32 (input_bfd, oldval | 0x000c0362,
			      contents + rel->r_offset);
		  r = bfd_reloc_ok;
		  goto done_reloc;
		}
	    }
	  /* Fall through.  */

	case R_C6000_PCR_S12:
	case R_C6000_PCR_S10:
	case R_C6000_PCR_S7:
	  if (h != NULL
	      && h->plt.offset != (bfd_vma) -1
	      && htab->elf.splt != NULL)
	    {
	      relocation = (htab->elf.splt->output_section->vma
			    + htab->elf.splt->output_offset
			    + h->plt.offset);
	    }

	  /* Generic PC-relative handling produces a value relative to
	     the exact location of the relocation.  Adjust it to be
	     relative to the start of the fetch packet instead.  */
	  relocation += (input_section->output_section->vma
			 + input_section->output_offset
			 + rel->r_offset) & 0x1f;
	  unresolved_reloc = false;
	  break;

	case R_C6000_PCR_H16:
	case R_C6000_PCR_L16:
	  off = (input_section->output_section->vma
		 + input_section->output_offset
		 + rel->r_offset);
	  /* These must be calculated as R = S - FP(FP(PC) - A).
	     PC, here, is the value we just computed in OFF.  RELOCATION
	     has the address of S + A. */
	  relocation -= rel->r_addend;
	  off2 = ((off & ~(bfd_vma)0x1f) - rel->r_addend) & (bfd_vma)~0x1f;
	  off2 = relocation - off2;
	  relocation = off + off2;
	  break;

	case R_C6000_DSBT_INDEX:
	  relocation = elf32_tic6x_hash_table (info)->params.dsbt_index;
	  if (!bfd_link_pic (info) || relocation != 0)
	    break;

	  /* fall through */
	case R_C6000_ABS32:
	case R_C6000_ABS16:
	case R_C6000_ABS8:
	case R_C6000_ABS_S16:
	case R_C6000_ABS_L16:
	case R_C6000_ABS_H16:
	  /* When generating a shared object or relocatable executable, these
	     relocations are copied into the output file to be resolved at
	     run time.  */
	  if ((bfd_link_pic (info) || elf32_tic6x_using_dsbt (output_bfd))
	      && (input_section->flags & SEC_ALLOC)
	      && (h == NULL
		  || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		  || h->root.type != bfd_link_hash_undefweak))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      asection *sreloc;

	      unresolved_reloc = false;

	      sreloc = elf_section_data (input_section)->sreloc;
	      BFD_ASSERT (sreloc != NULL && sreloc->contents != NULL);

	      skip = false;
	      relocate = false;

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
			   || !SYMBOLIC_BIND (info, h)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  long indx;

		  outrel.r_addend = relocation + rel->r_addend;

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

		      osec = sec->output_section;
		      indx = elf_section_data (osec)->dynindx;
		      outrel.r_addend -= osec->vma;
		      BFD_ASSERT (indx != 0);
		    }

		  outrel.r_info = ELF32_R_INFO (indx, r_type);
		}

	      elf32_tic6x_install_rela (output_bfd, sreloc, &outrel);

	      /* If this reloc is against an external symbol, we do not want to
		 fiddle with the addend.  Otherwise, we need to include the symbol
		 value so that it becomes an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }

	  /* Generic logic OK.  */
	  break;

	case R_C6000_SBR_U15_B:
	case R_C6000_SBR_U15_H:
	case R_C6000_SBR_U15_W:
	case R_C6000_SBR_S16:
	case R_C6000_SBR_L16_B:
	case R_C6000_SBR_L16_H:
	case R_C6000_SBR_L16_W:
	case R_C6000_SBR_H16_B:
	case R_C6000_SBR_H16_H:
	case R_C6000_SBR_H16_W:
	  sbh = bfd_link_hash_lookup (info->hash, "__c6xabi_DSBT_BASE",
				      false, false, true);
	  if (sbh != NULL
	      && (sbh->type == bfd_link_hash_defined
		  || sbh->type == bfd_link_hash_defweak))
	    {
	      if (h ? (h->root.type == bfd_link_hash_undefweak
		       && (htab->elf.splt == NULL
			   || h->plt.offset == (bfd_vma) -1))
		  : r_symndx != STN_UNDEF && bfd_is_und_section (sec))
		relocation = 0;
	      else
		relocation -= (sbh->u.def.value
			       + sbh->u.def.section->output_section->vma
			       + sbh->u.def.section->output_offset);
	    }
	  else
	    {
	      _bfd_error_handler (_("%pB: SB-relative relocation but "
				    "__c6xabi_DSBT_BASE not defined"),
				  input_bfd);
	      ok = false;
	      continue;
	    }
	  break;

	case R_C6000_SBR_GOT_U15_W:
	case R_C6000_SBR_GOT_L16_W:
	case R_C6000_SBR_GOT_H16_W:
	case R_C6000_EHTYPE:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  if (htab->elf.sgot == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      dyn = htab->elf.dynamic_sections_created;
	      if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     h)
		  || (bfd_link_pic (info)
		      && SYMBOL_REFERENCES_LOCAL (info, h))
		  || (ELF_ST_VISIBILITY (h->other)
		      && h->root.type == bfd_link_hash_undefweak))
		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple of 4, we use the
		     least significant bit to record whether we have
		     initialized it already.

		     When doing a dynamic link, we create a .rel.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  htab->elf.sgot->contents + off);
		      h->got.offset |= 1;

		      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							    bfd_link_pic (info),
							    h)
			  && !(ELF_ST_VISIBILITY (h->other)
			       && h->root.type == bfd_link_hash_undefweak))
			elf32_tic6x_make_got_dynreloc (output_bfd, htab, sec,
						       off);
		    }
		}
	      else
		unresolved_reloc = false;
	    }
	  else
	    {
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
		  bfd_put_32 (output_bfd, relocation,
			      htab->elf.sgot->contents + off);

		  if (bfd_link_pic (info) || elf32_tic6x_using_dsbt (output_bfd))
		    elf32_tic6x_make_got_dynreloc (output_bfd, htab, sec, off);

		  local_got_offsets[r_symndx] |= 1;
		}
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  if (htab->dsbt)
	    relocation = (htab->elf.sgot->output_section->vma
			  + htab->elf.sgot->output_offset + off
			  - htab->dsbt->output_section->vma
			  - htab->dsbt->output_offset);
	  else
	    relocation = (htab->elf.sgot->output_section->vma
			  + htab->elf.sgot->output_offset + off
			  - htab->elf.sgotplt->output_section->vma
			  - htab->elf.sgotplt->output_offset);

	  if (rel->r_addend != 0)
	    {
	      /* We can't do anything for a relocation which is against
		 a symbol *plus offset*.  GOT holds relocations for
		 symbols.  Make this an error; the compiler isn't
		 allowed to pass us these kinds of things.  */
	      if (h == NULL)
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB, section %pA: relocation %s with non-zero addend %"
		     PRId64 " against local symbol"),
		   input_bfd,
		   input_section,
		   elf32_tic6x_howto_table[r_type].name,
		   (int64_t) rel->r_addend);
	      else
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB, section %pA: relocation %s with non-zero addend %"
		     PRId64 " against symbol `%s'"),
		   input_bfd,
		   input_section,
		   elf32_tic6x_howto_table[r_type].name,
		   (int64_t) rel->r_addend,
		   h->root.root.string[0] != '\0' ? h->root.root.string
		   : _("[whose name is lost]"));

	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  break;

	case R_C6000_PREL31:
	  if (h != NULL
	      && h->plt.offset != (bfd_vma) -1
	      && htab->elf.splt != NULL)
	    {
	      relocation = (htab->elf.splt->output_section->vma
			    + htab->elf.splt->output_offset
			    + h->plt.offset);
	    }
	  break;

	case R_C6000_COPY:
	  /* Invalid in relocatable object.  */
	default:
	  /* Unknown relocation.  */
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ok = false;
	  continue;
	}

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

    done_reloc:
      if (r == bfd_reloc_ok
	  && howto->complain_on_overflow == complain_overflow_bitfield)
	{
	  /* Generic overflow handling accepts cases the ABI says
	     should be rejected for R_C6000_ABS16 and
	     R_C6000_ABS8.  */
	  bfd_vma value = (relocation + rel->r_addend) & 0xffffffff;
	  bfd_vma sbit = 1 << (howto->bitsize - 1);
	  bfd_vma sbits = (-(bfd_vma) sbit) & 0xffffffff;
	  bfd_vma value_sbits = value & sbits;

	  if (value_sbits != 0
	      && value_sbits != sbit
	      && value_sbits != sbits)
	    r = bfd_reloc_overflow;
	}

      if (r != bfd_reloc_ok)
	{
	  const char *name;
	  const char *error_message;

	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      sym->st_name);
	      if (name == NULL)
		return false;
	      if (*name == '\0')
		name = bfd_section_name (sec);
	    }

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
	      (*info->callbacks->undefined_symbol) (info, name, input_bfd,
						    input_section,
						    rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      error_message = _("out of range");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      error_message = _("unsupported relocation");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      error_message = _("dangerous relocation");
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

  return ok;
}


/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure linkage
   table, and dynamic reloc sections.  */

static bool
elf32_tic6x_check_relocs (bfd *abfd, struct bfd_link_info *info,
			  asection *sec, const Elf_Internal_Rela *relocs)
{
  struct elf32_tic6x_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;

  if (bfd_link_relocatable (info))
    return true;

  htab = elf32_tic6x_hash_table (info);
  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  /* Create dynamic sections for relocatable executables so that we can
     copy relocations.  */
  if ((bfd_link_pic (info) || elf32_tic6x_using_dsbt (abfd))
      && ! htab->elf.dynamic_sections_created)
    {
      if (! _bfd_elf_link_create_dynamic_sections (abfd, info))
	return false;
    }

  sreloc = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *isym;

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
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return false;
	  h = NULL;
	}
      else
	{
	  isym = NULL;
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      switch (r_type)
	{
	case R_C6000_PCR_S21:
	case R_C6000_PREL31:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code which is
	     never referenced by a dynamic object, in which case we
	     don't need to generate a procedure linkage table entry
	     after all.  */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (h == NULL)
	    continue;

	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  break;

	case R_C6000_SBR_GOT_U15_W:
	case R_C6000_SBR_GOT_L16_W:
	case R_C6000_SBR_GOT_H16_W:
	case R_C6000_EHTYPE:
	  /* This symbol requires a global offset table entry.  */
	  if (h != NULL)
	    {
	      h->got.refcount += 1;
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
		  size *= (sizeof (bfd_signed_vma)
			   + sizeof (bfd_vma) + sizeof(char));
		  local_got_refcounts = bfd_zalloc (abfd, size);
		  if (local_got_refcounts == NULL)
		    return false;
		  elf_local_got_refcounts (abfd) = local_got_refcounts;
		}
	      local_got_refcounts[r_symndx] += 1;
	    }

	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	  break;

	case R_C6000_DSBT_INDEX:
	  /* We'd like to check for nonzero dsbt_index here, but it's
	     set up only after check_relocs is called.  Instead, we
	     store the number of R_C6000_DSBT_INDEX relocs in the
	     pc_count field, and potentially discard the extra space
	     in elf32_tic6x_allocate_dynrelocs.  */
	  if (!bfd_link_pic (info))
	    break;

	  /* fall through */
	case R_C6000_ABS32:
	case R_C6000_ABS16:
	case R_C6000_ABS8:
	case R_C6000_ABS_S16:
	case R_C6000_ABS_L16:
	case R_C6000_ABS_H16:
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
	  if ((bfd_link_pic (info) || elf32_tic6x_using_dsbt (abfd))
	      && (sec->flags & SEC_ALLOC) != 0)
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      /* We must copy these reloc types into the output file.
		 Create a reloc section in dynobj and make room for
		 this reloc.  */
	      if (sreloc == NULL)
		{
		  if (htab->elf.dynobj == NULL)
		    htab->elf.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->elf.dynobj, 2, abfd, /*rela? */ true);

		  if (sreloc == NULL)
		    return false;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  head = &h->dyn_relocs;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */
		  void **vpp;
		  asection *s;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **)vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  size_t amt = sizeof *p;
		  p = bfd_alloc (htab->elf.dynobj, amt);
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (r_type == R_C6000_DSBT_INDEX)
		p->pc_count += 1;
	    }
	  break;

	case R_C6000_SBR_U15_B:
	case R_C6000_SBR_U15_H:
	case R_C6000_SBR_U15_W:
	case R_C6000_SBR_S16:
	case R_C6000_SBR_L16_B:
	case R_C6000_SBR_L16_H:
	case R_C6000_SBR_L16_W:
	case R_C6000_SBR_H16_B:
	case R_C6000_SBR_H16_H:
	case R_C6000_SBR_H16_W:
	  {
	    /* These relocations implicitly reference __c6xabi_DSBT_BASE.
	       Add an explicit reference so that the symbol will be
	       provided by a linker script.  */
	    struct bfd_link_hash_entry *bh = NULL;
	    if (!_bfd_generic_link_add_one_symbol (info, abfd,
						   "__c6xabi_DSBT_BASE",
						   BSF_GLOBAL,
						   bfd_und_section_ptr, 0,
						   NULL, false, false, &bh))
	      return false;
	    ((struct elf_link_hash_entry *) bh)->non_elf = 0;
	  }
	  if (h != NULL && bfd_link_executable (info))
	    {
	      /* For B14-relative addresses, we might need a copy
		 reloc.  */
	      h->non_got_ref = 1;
	    }
	  break;

	default:
	  break;
	}
    }

  return true;
}

static bool
elf32_tic6x_add_symbol_hook (bfd *abfd,
			     struct bfd_link_info *info ATTRIBUTE_UNUSED,
			     Elf_Internal_Sym *sym,
			     const char **namep ATTRIBUTE_UNUSED,
			     flagword *flagsp ATTRIBUTE_UNUSED,
			     asection **secp,
			     bfd_vma *valp)
{
  switch (sym->st_shndx)
    {
    case SHN_TIC6X_SCOMMON:
      *secp = bfd_make_section_old_way (abfd, ".scommon");
      (*secp)->flags |= SEC_IS_COMMON | SEC_SMALL_DATA;
      *valp = sym->st_size;
      bfd_set_section_alignment (*secp, bfd_log2 (sym->st_value));
      break;
    }

  return true;
}

static void
elf32_tic6x_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED, asymbol *asym)
{
  elf_symbol_type *elfsym;

  elfsym = (elf_symbol_type *) asym;
  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_TIC6X_SCOMMON:
      asym->section = &tic6x_elf_scom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    }
}

static int
elf32_tic6x_link_output_symbol_hook (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				     const char *name ATTRIBUTE_UNUSED,
				     Elf_Internal_Sym *sym,
				     asection *input_sec,
				     struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (sym->st_shndx == SHN_COMMON && strcmp (input_sec->name, ".scommon") == 0)
    sym->st_shndx = SHN_TIC6X_SCOMMON;

  return 1;
}

static bool
elf32_tic6x_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
				      asection *sec,
				      int *retval)
{
  if (strcmp (bfd_section_name (sec), ".scommon") == 0)
    {
      *retval = SHN_TIC6X_SCOMMON;
      return true;
    }

  return false;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
elf32_tic6x_allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf32_tic6x_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = elf32_tic6x_hash_table (info);

  if (htab->elf.dynamic_sections_created && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      if (bfd_link_pic (info)
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->elf.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += PLT_ENTRY_SIZE;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info) && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += PLT_ENTRY_SIZE;
	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->elf.sgotplt->size += 4;
	  /* We also need to make an entry in the .rel.plt section.  */
	  htab->elf.srelplt->size += sizeof (Elf32_External_Rela);
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
      asection *s;

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
      s->size += 4;

      if (!(ELF_ST_VISIBILITY (h->other)
	    && h->root.type == bfd_link_hash_undefweak))
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  if (h->dyn_relocs == NULL)
    return true;

  /* Discard relocs on undefined weak syms with non-default
     visibility.  */
  if (bfd_link_pic (info) || elf32_tic6x_using_dsbt (htab->obfd))
    {
      /* We use the pc_count field to hold the number of
	 R_C6000_DSBT_INDEX relocs.  */
      if (htab->params.dsbt_index != 0)
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

  /* Finally, allocate space.  */
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc;

      sreloc = elf_section_data (p->sec)->sreloc;

      BFD_ASSERT (sreloc != NULL);
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
elf32_tic6x_size_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  struct elf32_tic6x_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = elf32_tic6x_hash_table (info);
  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    abort ();

  if (htab->elf.dynamic_sections_created)
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

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = ((struct elf_dyn_relocs *)
		     elf_section_data (s)->local_dynrel);
	       p != NULL;
	       p = p->next)
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
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->elf.sgot;
      srel = htab->elf.srelgot;
      for (; local_got < end_local_got; ++local_got)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += 4;

	      if (bfd_link_pic (info) || elf32_tic6x_using_dsbt (output_bfd))
		{
		  srel->size += sizeof (Elf32_External_Rela);
		}
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, elf32_tic6x_allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      bool strip_section = true;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->dsbt)
	s->size = 4 * htab->params.dsbt_size;
      else if (s == htab->elf.splt
	       || s == htab->elf.sgot
	       || s == htab->elf.sgotplt
	       || s == htab->elf.sdynbss
	       || s == htab->elf.sdynrelro)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	  /* We'd like to strip these sections if they aren't needed, but if
	     we've exported dynamic symbols from them we must leave them.
	     It's too late to tell BFD to get rid of the symbols.  */

	  if (htab->elf.hplt != NULL)
	    strip_section = false;

	  /* Round up the size of the PLT section to a multiple of 32.  */
	  if (s == htab->elf.splt && s->size > 0)
	    s->size = (s->size + 31) & ~(bfd_vma)31;
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0
	      && s != htab->elf.srelplt)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rel.bss and
	     .rel.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */
	  if (strip_section)
	    s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_C6000_NONE reloc instead
	 of garbage.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in elf32_tic6x_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (!_bfd_elf_add_dynamic_tags (output_bfd, info, relocs))
	return false;

      if (!add_dynamic_entry (DT_C6000_DSBT_BASE, 0)
	  || !add_dynamic_entry (DT_C6000_DSBT_SIZE, htab->params.dsbt_size)
	  || !add_dynamic_entry (DT_C6000_DSBT_INDEX,
				 htab->params.dsbt_index))
	return false;

    }
#undef add_dynamic_entry

  return true;
}

/* This function is called after all the input files have been read,
   and the input sections have been assigned to output sections.  */

static bool
elf32_tic6x_always_size_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  if (elf32_tic6x_using_dsbt (output_bfd) && !bfd_link_relocatable (info)
      && !bfd_elf_stack_segment_size (output_bfd, info,
				      "__stacksize", DEFAULT_STACK_SIZE))
    return false;

  return true;
}

static bool
elf32_tic6x_finish_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				     struct bfd_link_info *info)
{
  struct elf32_tic6x_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = elf32_tic6x_hash_table (info);
  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf32_External_Dyn * dyncon;
      Elf32_External_Dyn * dynconend;

      BFD_ASSERT (sdyn != NULL);

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
	      break;

	    case DT_C6000_DSBT_BASE:
	      s = htab->dsbt;
	      dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset);
	      break;

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
	    }
	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}

      /* Fill in the first entry in the procedure linkage table.  */
      if (htab->elf.splt && htab->elf.splt->size > 0)
	{
	  bfd_vma got_offs = (htab->elf.sgotplt->output_section->vma
			      + htab->elf.sgotplt->output_offset
			      - htab->dsbt->output_section->vma
			      - htab->dsbt->output_offset) / 4;

	  /* ldw .D2T2 *+b14[$GOT(0)],b2 */
	  bfd_put_32 (output_bfd, got_offs << 8 | 0x0100006e,
		      htab->elf.splt->contents);
	  /* ldw .D2T2 *+b14[$GOT(4)],b1 */
	  bfd_put_32 (output_bfd, (got_offs + 1) << 8 | 0x0080006e,
		      htab->elf.splt->contents + 4);
	  /* nop 3 */
	  bfd_put_32 (output_bfd, 0x00004000,
		      htab->elf.splt->contents + 8);
	  /* b .s2 b2 */
	  bfd_put_32 (output_bfd, 0x00080362,
		      htab->elf.splt->contents + 12);
	  /* nop 5 */
	  bfd_put_32 (output_bfd, 0x00008000,
		      htab->elf.splt->contents + 16);

	  elf_section_data (htab->elf.splt->output_section)
	    ->this_hdr.sh_entsize = PLT_ENTRY_SIZE;
	}
    }

  return true;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elf32_tic6x_plt_sym_val (bfd_vma i, const asection *plt,
			 const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * PLT_ENTRY_SIZE;
}

static int
elf32_tic6x_obj_attrs_arg_type (int tag)
{
  if (tag == Tag_ABI_compatibility)
    return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL;
  else if (tag & 1)
    return ATTR_TYPE_FLAG_STR_VAL;
  else
    return ATTR_TYPE_FLAG_INT_VAL;
}

static int
elf32_tic6x_obj_attrs_order (int num)
{
  if (num == LEAST_KNOWN_OBJ_ATTRIBUTE)
    return Tag_ABI_conformance;
  if ((num - 1) < Tag_ABI_conformance)
    return num - 1;
  return num;
}

static bool
elf32_tic6x_obj_attrs_handle_unknown (bfd *abfd, int tag)
{
  if ((tag & 127) < 64)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: error: unknown mandatory EABI object attribute %d"),
	 abfd, tag);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: warning: unknown EABI object attribute %d"),
	 abfd, tag);
      return true;
    }
}

/* Merge the Tag_ISA attribute values ARCH1 and ARCH2
   and return the merged value.  At present, all merges succeed, so no
   return value for errors is defined.  */

int
elf32_tic6x_merge_arch_attributes (int arch1, int arch2)
{
  int min_arch, max_arch;

  min_arch = (arch1 < arch2 ? arch1 : arch2);
  max_arch = (arch1 > arch2 ? arch1 : arch2);

  /* In most cases, the numerically greatest value is the correct
     merged value, but merging C64 and C67 results in C674X.  */
  if ((min_arch == C6XABI_Tag_ISA_C67X
       || min_arch == C6XABI_Tag_ISA_C67XP)
      && (max_arch == C6XABI_Tag_ISA_C64X
	  || max_arch == C6XABI_Tag_ISA_C64XP))
    return C6XABI_Tag_ISA_C674X;

  return max_arch;
}

/* Convert a Tag_ABI_array_object_alignment or
   Tag_ABI_array_object_align_expected tag value TAG to a
   corresponding alignment value; return the alignment, or -1 for an
   unknown tag value.  */

static int
elf32_tic6x_tag_to_array_alignment (int tag)
{
  switch (tag)
    {
    case 0:
      return 8;

    case 1:
      return 4;

    case 2:
      return 16;

    default:
      return -1;
    }
}

/* Convert a Tag_ABI_array_object_alignment or
   Tag_ABI_array_object_align_expected alignment ALIGN to a
   corresponding tag value; return the tag value.  */

static int
elf32_tic6x_array_alignment_to_tag (int align)
{
  switch (align)
    {
    case 8:
      return 0;

    case 4:
      return 1;

    case 16:
      return 2;

    default:
      abort ();
    }
}

/* Merge attributes from IBFD and OBFD, returning TRUE if the merge
   succeeded, FALSE otherwise.  */

static bool
elf32_tic6x_merge_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  bool result = true;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  int i;
  int array_align_in, array_align_out, array_expect_in, array_expect_out;

  /* FIXME: What should be checked when linking shared libraries?  */
  if ((ibfd->flags & DYNAMIC) != 0)
    return true;

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      out_attr = elf_known_obj_attributes_proc (obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      out_attr[0].i = 1;

      return true;
    }

  in_attr = elf_known_obj_attributes_proc (ibfd);
  out_attr = elf_known_obj_attributes_proc (obfd);

  /* No specification yet for handling of unknown attributes, so just
     ignore them and handle known ones.  */

  if (out_attr[Tag_ABI_stack_align_preserved].i
      < in_attr[Tag_ABI_stack_align_needed].i)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB requires more stack alignment than %pB preserves"),
	 ibfd, obfd);
      result = false;
    }
  if (in_attr[Tag_ABI_stack_align_preserved].i
      < out_attr[Tag_ABI_stack_align_needed].i)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB requires more stack alignment than %pB preserves"),
	 obfd, ibfd);
      result = false;
    }

  array_align_in = elf32_tic6x_tag_to_array_alignment
    (in_attr[Tag_ABI_array_object_alignment].i);
  if (array_align_in == -1)
    {
      _bfd_error_handler
	(_("error: unknown Tag_ABI_array_object_alignment value in %pB"),
	 ibfd);
      result = false;
    }
  array_align_out = elf32_tic6x_tag_to_array_alignment
    (out_attr[Tag_ABI_array_object_alignment].i);
  if (array_align_out == -1)
    {
      _bfd_error_handler
	(_("error: unknown Tag_ABI_array_object_alignment value in %pB"),
	 obfd);
      result = false;
    }
  array_expect_in = elf32_tic6x_tag_to_array_alignment
    (in_attr[Tag_ABI_array_object_align_expected].i);
  if (array_expect_in == -1)
    {
      _bfd_error_handler
	(_("error: unknown Tag_ABI_array_object_align_expected value in %pB"),
	 ibfd);
      result = false;
    }
  array_expect_out = elf32_tic6x_tag_to_array_alignment
    (out_attr[Tag_ABI_array_object_align_expected].i);
  if (array_expect_out == -1)
    {
      _bfd_error_handler
	(_("error: unknown Tag_ABI_array_object_align_expected value in %pB"),
	 obfd);
      result = false;
    }

  if (array_align_out < array_expect_in)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB requires more array alignment than %pB preserves"),
	 ibfd, obfd);
      result = false;
    }
  if (array_align_in < array_expect_out)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("error: %pB requires more array alignment than %pB preserves"),
	 obfd, ibfd);
      result = false;
    }

  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    {
      switch (i)
	{
	case Tag_ISA:
	  out_attr[i].i = elf32_tic6x_merge_arch_attributes (in_attr[i].i,
							     out_attr[i].i);
	  break;

	case Tag_ABI_wchar_t:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  if (out_attr[i].i != 0
	      && in_attr[i].i != 0
	      && out_attr[i].i != in_attr[i].i)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("warning: %pB and %pB differ in wchar_t size"), obfd, ibfd);
	    }
	  break;

	case Tag_ABI_stack_align_needed:
	  if (out_attr[i].i < in_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_ABI_stack_align_preserved:
	  if (out_attr[i].i > in_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_ABI_DSBT:
	  if (out_attr[i].i != in_attr[i].i)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("warning: %pB and %pB differ in whether code is "
		   "compiled for DSBT"),
		 obfd, ibfd);
	    }
	  break;

	case Tag_ABI_PIC:
	case Tag_ABI_PID:
	  /* Don't transfer these tags from dynamic objects.  */
	  if ((ibfd->flags & DYNAMIC) != 0)
	    continue;
	  if (out_attr[i].i > in_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_ABI_array_object_alignment:
	  if (array_align_out != -1
	      && array_align_in != -1
	      && array_align_out > array_align_in)
	    out_attr[i].i
	      = elf32_tic6x_array_alignment_to_tag (array_align_in);
	  break;

	case Tag_ABI_array_object_align_expected:
	  if (array_expect_out != -1
	      && array_expect_in != -1
	      && array_expect_out < array_expect_in)
	    out_attr[i].i
	      = elf32_tic6x_array_alignment_to_tag (array_expect_in);
	  break;

	case Tag_ABI_conformance:
	  /* Merging for this attribute is not specified.  As on ARM,
	     treat a missing attribute as no claim to conform and only
	     merge identical values.  */
	  if (out_attr[i].s == NULL
	      || in_attr[i].s == NULL
	      || strcmp (out_attr[i].s,
			 in_attr[i].s) != 0)
	    out_attr[i].s = NULL;
	  break;

	case Tag_ABI_compatibility:
	  /* Merged in _bfd_elf_merge_object_attributes.  */
	  break;

	default:
	  result
	    = result && _bfd_elf_merge_unknown_attribute_low (ibfd, obfd, i);
	  break;
	}

      if (in_attr[i].type && !out_attr[i].type)
	out_attr[i].type = in_attr[i].type;
    }

  /* Merge Tag_ABI_compatibility attributes and any common GNU ones.  */
  if (!_bfd_elf_merge_object_attributes (ibfd, info))
    return false;

  result &= _bfd_elf_merge_unknown_attribute_list (ibfd, obfd);

  return result;
}

static bool
elf32_tic6x_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  if (!_bfd_generic_verify_endian_match (ibfd, info))
    return false;

  if (! is_tic6x_elf (ibfd) || ! is_tic6x_elf (info->output_bfd))
    return true;

  if (!elf32_tic6x_merge_attributes (ibfd, info))
    return false;

  return true;
}

/* Add a new unwind edit to the list described by HEAD, TAIL.  If TINDEX is zero,
   adds the edit to the start of the list.  (The list must be built in order of
   ascending TINDEX: the function's callers are primarily responsible for
   maintaining that condition).  */

static void
elf32_tic6x_add_unwind_table_edit (tic6x_unwind_table_edit **head,
				   tic6x_unwind_table_edit **tail,
				   tic6x_unwind_edit_type type,
				   asection *linked_section,
				   unsigned int tindex)
{
  tic6x_unwind_table_edit *new_edit = (tic6x_unwind_table_edit *)
      xmalloc (sizeof (tic6x_unwind_table_edit));

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

static _tic6x_elf_section_data *
get_tic6x_elf_section_data (asection * sec)
{
  if (sec && sec->owner && is_tic6x_elf (sec->owner))
    return elf32_tic6x_section_data (sec);
  else
    return NULL;
}


/* Increase the size of EXIDX_SEC by ADJUST bytes.  ADJUST must be negative.  */
static void
elf32_tic6x_adjust_exidx_size (asection *exidx_sec, int adjust)
{
  asection *out_sec;

  if (!exidx_sec->rawsize)
    exidx_sec->rawsize = exidx_sec->size;

  bfd_set_section_size (exidx_sec, exidx_sec->size + adjust);
  out_sec = exidx_sec->output_section;
  /* Adjust size of output section.  */
  bfd_set_section_size (out_sec, out_sec->size +adjust);
}

/* Insert an EXIDX_CANTUNWIND marker at the end of a section.  */
static void
elf32_tic6x_insert_cantunwind_after (asection *text_sec, asection *exidx_sec)
{
  struct _tic6x_elf_section_data *exidx_data;

  exidx_data = get_tic6x_elf_section_data (exidx_sec);
  elf32_tic6x_add_unwind_table_edit (
    &exidx_data->u.exidx.unwind_edit_list,
    &exidx_data->u.exidx.unwind_edit_tail,
    INSERT_EXIDX_CANTUNWIND_AT_END, text_sec, UINT_MAX);

  elf32_tic6x_adjust_exidx_size (exidx_sec, 8);
}

/* Scan .cx6abi.exidx tables, and create a list describing edits which
   should be made to those tables, such that:

     1. Regions without unwind data are marked with EXIDX_CANTUNWIND entries.
     2. Duplicate entries are merged together (EXIDX_CANTUNWIND, or unwind
	codes which have been inlined into the index).

   If MERGE_EXIDX_ENTRIES is false, duplicate entries are not merged.

   The edits are applied when the tables are written
   (in elf32_tic6x_write_section).
*/

bool
elf32_tic6x_fix_exidx_coverage (asection **text_section_order,
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

	  if (!hdr || hdr->sh_type != SHT_C6000_UNWIND)
	    continue;

	  if (elf_sec->linked_to)
	    {
	      Elf_Internal_Shdr *linked_hdr
		= &elf_section_data (elf_sec->linked_to)->this_hdr;
	      struct _tic6x_elf_section_data *linked_sec_tic6x_data
		= get_tic6x_elf_section_data (linked_hdr->bfd_section);

	      if (linked_sec_tic6x_data == NULL)
		continue;

	      /* Link this .c6xabi.exidx section back from the
		 text section it describes.  */
	      linked_sec_tic6x_data->u.text.tic6x_exidx_sec = sec;
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
      struct _tic6x_elf_section_data *tic6x_data
	= get_tic6x_elf_section_data (sec);
      struct _tic6x_elf_section_data *exidx_data;
      bfd_byte *contents = NULL;
      int deleted_exidx_bytes = 0;
      bfd_vma j;
      tic6x_unwind_table_edit *unwind_edit_head = NULL;
      tic6x_unwind_table_edit *unwind_edit_tail = NULL;
      Elf_Internal_Shdr *hdr;
      bfd *ibfd;

      if (tic6x_data == NULL)
	continue;

      exidx_sec = tic6x_data->u.text.tic6x_exidx_sec;
      if (exidx_sec == NULL)
	{
	  /* Section has no unwind data.  */
	  if (last_unwind_type == 0 || !last_exidx_sec)
	    continue;

	  /* Ignore zero sized sections.  */
	  if (sec->size == 0)
	    continue;

	  elf32_tic6x_insert_cantunwind_after (last_text_sec, last_exidx_sec);
	  last_unwind_type = 0;
	  continue;
	}

      /* Skip /DISCARD/ sections.  */
      if (bfd_is_abs_section (exidx_sec->output_section))
	continue;

      hdr = &elf_section_data (exidx_sec)->this_hdr;
      if (hdr->sh_type != SHT_C6000_UNWIND)
	continue;

      exidx_data = get_tic6x_elf_section_data (exidx_sec);
      if (exidx_data == NULL)
	continue;

      ibfd = exidx_sec->owner;

      if (hdr->contents != NULL)
	contents = hdr->contents;
      else if (! bfd_malloc_and_get_section (ibfd, exidx_sec, &contents))
	/* An error?  */
	continue;

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
		  && last_second_word == second_word
		  && last_unwind_type == 1)
		elide = 1;
	      unwind_type = 1;
	      last_second_word = second_word;
	    }
	  /* Normal table entry.  In theory we could merge these too,
	     but duplicate entries are likely to be much less common.  */
	  else
	    unwind_type = 2;

	  if (elide)
	    {
	      elf32_tic6x_add_unwind_table_edit (&unwind_edit_head,
		  &unwind_edit_tail, DELETE_EXIDX_ENTRY, NULL, j / 8);

	      deleted_exidx_bytes += 8;
	    }

	  last_unwind_type = unwind_type;
	}

      /* Free contents if we allocated it ourselves.  */
      if (contents != hdr->contents)
	free (contents);

      /* Record edits to be applied later (in elf32_tic6x_write_section).  */
      exidx_data->u.exidx.unwind_edit_list = unwind_edit_head;
      exidx_data->u.exidx.unwind_edit_tail = unwind_edit_tail;

      if (deleted_exidx_bytes > 0)
	elf32_tic6x_adjust_exidx_size (exidx_sec, -deleted_exidx_bytes);

      last_exidx_sec = exidx_sec;
      last_text_sec = sec;
    }

  /* Add terminating CANTUNWIND entry.  */
  if (last_exidx_sec && last_unwind_type != 0)
    elf32_tic6x_insert_cantunwind_after (last_text_sec, last_exidx_sec);

  return true;
}

/* Add ADDEND to lower 31 bits of VAL, leaving other bits unmodified.  */

static unsigned long
elf32_tic6x_add_low31 (unsigned long val, bfd_vma addend)
{
  return (val & ~0x7ffffffful) | ((val + addend) & 0x7ffffffful);
}

/* Copy an .c6xabi.exidx table entry, adding OFFSET to (applied) PREL31
   relocations.  OFFSET is in bytes, and will be scaled before encoding.  */


static void
elf32_tic6x_copy_exidx_entry (bfd *output_bfd, bfd_byte *to, bfd_byte *from,
			      bfd_vma offset)
{
  unsigned long first_word = bfd_get_32 (output_bfd, from);
  unsigned long second_word = bfd_get_32 (output_bfd, from + 4);

  offset >>= 1;
  /* High bit of first word is supposed to be zero.  */
  if ((first_word & 0x80000000ul) == 0)
    first_word = elf32_tic6x_add_low31 (first_word, offset);

  /* If the high bit of the first word is clear, and the bit pattern is not 0x1
     (EXIDX_CANTUNWIND), this is an offset to an .c6xabi.extab entry.  */
  if ((second_word != 0x1) && ((second_word & 0x80000000ul) == 0))
    second_word = elf32_tic6x_add_low31 (second_word, offset);

  bfd_put_32 (output_bfd, first_word, to);
  bfd_put_32 (output_bfd, second_word, to + 4);
}

/* Do the actual mangling of exception index tables.  */

static bool
elf32_tic6x_write_section (bfd *output_bfd,
			 struct bfd_link_info *link_info,
			 asection *sec,
			 bfd_byte *contents)
{
  _tic6x_elf_section_data *tic6x_data;
  struct elf32_tic6x_link_hash_table *globals
    = elf32_tic6x_hash_table (link_info);
  bfd_vma offset = sec->output_section->vma + sec->output_offset;

  if (globals == NULL)
    return false;

  /* If this section has not been allocated an _tic6x_elf_section_data
     structure then we cannot record anything.  */
  tic6x_data = get_tic6x_elf_section_data (sec);
  if (tic6x_data == NULL)
    return false;

  if (tic6x_data->elf.this_hdr.sh_type != SHT_C6000_UNWIND)
    return false;

  tic6x_unwind_table_edit *edit_node
    = tic6x_data->u.exidx.unwind_edit_list;
  /* Now, sec->size is the size of the section we will write.  The original
     size (before we merged duplicate entries and inserted EXIDX_CANTUNWIND
     markers) was sec->rawsize.  (This isn't the case if we perform no
     edits, then rawsize will be zero and we should use size).  */
  bfd_byte *edited_contents = (bfd_byte *) bfd_malloc (sec->size);
  unsigned int input_size = sec->rawsize ? sec->rawsize : sec->size;
  unsigned int in_index, out_index;
  bfd_vma add_to_offsets = 0;

  for (in_index = 0, out_index = 0; in_index * 8 < input_size || edit_node;)
    {
      if (edit_node)
	{
	  unsigned int edit_index = edit_node->index;

	  if (in_index < edit_index && in_index * 8 < input_size)
	    {
	      elf32_tic6x_copy_exidx_entry (output_bfd,
		  edited_contents + out_index * 8,
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
		       R_C6000_PREL31 relocation.  These synthetic
		       EXIDX_CANTUNWIND markers are not relocated by the
		       usual BFD method.  */
		    prel31_offset = ((text_offset - exidx_offset) >> 1)
				    & 0x7ffffffful;

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
	  elf32_tic6x_copy_exidx_entry (output_bfd,
	      edited_contents + out_index * 8,
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

#define	elf32_bed		elf32_tic6x_bed

#define TARGET_LITTLE_SYM	tic6x_elf32_le_vec
#define TARGET_LITTLE_NAME	"elf32-tic6x-le"
#define TARGET_BIG_SYM		tic6x_elf32_be_vec
#define TARGET_BIG_NAME		"elf32-tic6x-be"
#define ELF_ARCH		bfd_arch_tic6x
#define ELF_TARGET_ID		TIC6X_ELF_DATA
#define ELF_MACHINE_CODE	EM_TI_C6000
#define ELF_MAXPAGESIZE		0x1000
#define bfd_elf32_bfd_reloc_type_lookup elf32_tic6x_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup elf32_tic6x_reloc_name_lookup
#define bfd_elf32_bfd_merge_private_bfd_data	elf32_tic6x_merge_private_bfd_data
#define bfd_elf32_mkobject		elf32_tic6x_mkobject
#define bfd_elf32_bfd_link_hash_table_create  elf32_tic6x_link_hash_table_create
#define bfd_elf32_new_section_hook	elf32_tic6x_new_section_hook
#define elf_backend_stack_align		8
#define elf_backend_can_gc_sections	1
#define elf_backend_default_use_rela_p	1
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	1
#define elf_backend_obj_attrs_arg_type	elf32_tic6x_obj_attrs_arg_type
#define elf_backend_obj_attrs_handle_unknown	elf32_tic6x_obj_attrs_handle_unknown
#define elf_backend_obj_attrs_order	elf32_tic6x_obj_attrs_order
#define elf_backend_obj_attrs_section	".c6xabi.attributes"
#define elf_backend_obj_attrs_section_type	SHT_C6000_ATTRIBUTES
#define elf_backend_obj_attrs_vendor	"c6xabi"
#define elf_backend_can_refcount	1
#define elf_backend_want_got_plt	1
#define elf_backend_want_dynbss		1
#define elf_backend_want_dynrelro	1
#define elf_backend_plt_readonly	1
#define elf_backend_rela_normal		1
#define elf_backend_got_header_size     8
#define elf_backend_fake_sections       elf32_tic6x_fake_sections
#define elf_backend_gc_mark_extra_sections elf32_tic6x_gc_mark_extra_sections
#define elf_backend_create_dynamic_sections \
  elf32_tic6x_create_dynamic_sections
#define elf_backend_adjust_dynamic_symbol \
  elf32_tic6x_adjust_dynamic_symbol
#define elf_backend_check_relocs	elf32_tic6x_check_relocs
#define elf_backend_add_symbol_hook     elf32_tic6x_add_symbol_hook
#define elf_backend_symbol_processing   elf32_tic6x_symbol_processing
#define elf_backend_link_output_symbol_hook \
  elf32_tic6x_link_output_symbol_hook
#define elf_backend_section_from_bfd_section \
  elf32_tic6x_section_from_bfd_section
#define elf_backend_relocate_section	elf32_tic6x_relocate_section
#define elf_backend_relocs_compatible	_bfd_elf_relocs_compatible
#define elf_backend_finish_dynamic_symbol \
  elf32_tic6x_finish_dynamic_symbol
#define elf_backend_always_size_sections \
  elf32_tic6x_always_size_sections
#define elf_backend_size_dynamic_sections \
  elf32_tic6x_size_dynamic_sections
#define elf_backend_finish_dynamic_sections \
  elf32_tic6x_finish_dynamic_sections
#define bfd_elf32_bfd_final_link \
	elf32_tic6x_final_link
#define elf_backend_write_section	elf32_tic6x_write_section
#define elf_info_to_howto		elf32_tic6x_info_to_howto
#define elf_info_to_howto_rel		elf32_tic6x_info_to_howto_rel

#undef elf_backend_omit_section_dynsym
#define elf_backend_omit_section_dynsym elf32_tic6x_link_omit_section_dynsym
#define elf_backend_plt_sym_val		elf32_tic6x_plt_sym_val

#include "elf32-target.h"

#undef elf32_bed
#define	elf32_bed		elf32_tic6x_linux_bed

#undef TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		tic6x_elf32_linux_le_vec
#undef TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-tic6x-linux-le"
#undef TARGET_BIG_SYM
#define TARGET_BIG_SYM			tic6x_elf32_linux_be_vec
#undef TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-tic6x-linux-be"
#undef ELF_OSABI
#define	ELF_OSABI			ELFOSABI_C6000_LINUX

#include "elf32-target.h"

#undef elf32_bed
#define	elf32_bed		elf32_tic6x_elf_bed

#undef TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		tic6x_elf32_c6000_le_vec
#undef TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-tic6x-elf-le"
#undef TARGET_BIG_SYM
#define TARGET_BIG_SYM			tic6x_elf32_c6000_be_vec
#undef TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-tic6x-elf-be"
#undef ELF_OSABI
#define	ELF_OSABI			ELFOSABI_C6000_ELFABI

#include "elf32-target.h"
