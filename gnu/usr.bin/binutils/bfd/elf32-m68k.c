/* Motorola 68k series support for 32-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/m68k.h"
#include "opcode/m68k.h"
#include "cpu-m68k.h"
#include "elf32-m68k.h"

static bool
elf_m68k_discard_copies (struct elf_link_hash_entry *, void *);

static reloc_howto_type howto_table[] =
{
  HOWTO(R_68K_NONE,	  0, 0, 0, false,0, complain_overflow_dont,	bfd_elf_generic_reloc, "R_68K_NONE",	  false, 0, 0x00000000,false),
  HOWTO(R_68K_32,	  0, 4,32, false,0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_32",	  false, 0, 0xffffffff,false),
  HOWTO(R_68K_16,	  0, 2,16, false,0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_16",	  false, 0, 0x0000ffff,false),
  HOWTO(R_68K_8,	  0, 1, 8, false,0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_8",	  false, 0, 0x000000ff,false),
  HOWTO(R_68K_PC32,	  0, 4,32, true, 0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_PC32",	  false, 0, 0xffffffff,true),
  HOWTO(R_68K_PC16,	  0, 2,16, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PC16",	  false, 0, 0x0000ffff,true),
  HOWTO(R_68K_PC8,	  0, 1, 8, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PC8",	  false, 0, 0x000000ff,true),
  HOWTO(R_68K_GOT32,	  0, 4,32, true, 0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_GOT32",	  false, 0, 0xffffffff,true),
  HOWTO(R_68K_GOT16,	  0, 2,16, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_GOT16",	  false, 0, 0x0000ffff,true),
  HOWTO(R_68K_GOT8,	  0, 1, 8, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_GOT8",	  false, 0, 0x000000ff,true),
  HOWTO(R_68K_GOT32O,	  0, 4,32, false,0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_GOT32O",	  false, 0, 0xffffffff,false),
  HOWTO(R_68K_GOT16O,	  0, 2,16, false,0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_GOT16O",	  false, 0, 0x0000ffff,false),
  HOWTO(R_68K_GOT8O,	  0, 1, 8, false,0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_GOT8O",	  false, 0, 0x000000ff,false),
  HOWTO(R_68K_PLT32,	  0, 4,32, true, 0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_PLT32",	  false, 0, 0xffffffff,true),
  HOWTO(R_68K_PLT16,	  0, 2,16, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PLT16",	  false, 0, 0x0000ffff,true),
  HOWTO(R_68K_PLT8,	  0, 1, 8, true, 0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PLT8",	  false, 0, 0x000000ff,true),
  HOWTO(R_68K_PLT32O,	  0, 4,32, false,0, complain_overflow_bitfield, bfd_elf_generic_reloc, "R_68K_PLT32O",	  false, 0, 0xffffffff,false),
  HOWTO(R_68K_PLT16O,	  0, 2,16, false,0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PLT16O",	  false, 0, 0x0000ffff,false),
  HOWTO(R_68K_PLT8O,	  0, 1, 8, false,0, complain_overflow_signed,	bfd_elf_generic_reloc, "R_68K_PLT8O",	  false, 0, 0x000000ff,false),
  HOWTO(R_68K_COPY,	  0, 0, 0, false,0, complain_overflow_dont,	bfd_elf_generic_reloc, "R_68K_COPY",	  false, 0, 0xffffffff,false),
  HOWTO(R_68K_GLOB_DAT,	  0, 4,32, false,0, complain_overflow_dont,	bfd_elf_generic_reloc, "R_68K_GLOB_DAT",  false, 0, 0xffffffff,false),
  HOWTO(R_68K_JMP_SLOT,	  0, 4,32, false,0, complain_overflow_dont,	bfd_elf_generic_reloc, "R_68K_JMP_SLOT",  false, 0, 0xffffffff,false),
  HOWTO(R_68K_RELATIVE,	  0, 4,32, false,0, complain_overflow_dont,	bfd_elf_generic_reloc, "R_68K_RELATIVE",  false, 0, 0xffffffff,false),
  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_68K_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_68K_GNU_VTINHERIT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),
  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_68K_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_68K_GNU_VTENTRY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),

  /* TLS general dynamic variable reference.  */
  HOWTO (R_68K_TLS_GD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_GD32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_GD16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_GD16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_GD8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_GD8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic variable reference.  */
  HOWTO (R_68K_TLS_LDM32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDM32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LDM16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDM16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LDM8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDM8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LDO32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDO32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LDO16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDO16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LDO8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LDO8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS initial execution variable reference.  */
  HOWTO (R_68K_TLS_IE32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_IE32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_IE16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_IE16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_IE8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_IE8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local execution variable reference.  */
  HOWTO (R_68K_TLS_LE32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LE32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LE16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LE16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_LE8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_LE8",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS GD/LD dynamic relocations.  */
  HOWTO (R_68K_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_DTPMOD32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_DTPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_DTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_68K_TLS_TPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_68K_TLS_TPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
};

static bool
rtype_to_howto (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int indx = ELF32_R_TYPE (dst->r_info);

  if (indx >= (unsigned int) R_68K_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, indx);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &howto_table[indx];
  return true;
}

#define elf_info_to_howto rtype_to_howto

static const struct
{
  bfd_reloc_code_real_type bfd_val;
  int elf_val;
}
  reloc_map[] =
{
  { BFD_RELOC_NONE, R_68K_NONE },
  { BFD_RELOC_32, R_68K_32 },
  { BFD_RELOC_16, R_68K_16 },
  { BFD_RELOC_8, R_68K_8 },
  { BFD_RELOC_32_PCREL, R_68K_PC32 },
  { BFD_RELOC_16_PCREL, R_68K_PC16 },
  { BFD_RELOC_8_PCREL, R_68K_PC8 },
  { BFD_RELOC_32_GOT_PCREL, R_68K_GOT32 },
  { BFD_RELOC_16_GOT_PCREL, R_68K_GOT16 },
  { BFD_RELOC_8_GOT_PCREL, R_68K_GOT8 },
  { BFD_RELOC_32_GOTOFF, R_68K_GOT32O },
  { BFD_RELOC_16_GOTOFF, R_68K_GOT16O },
  { BFD_RELOC_8_GOTOFF, R_68K_GOT8O },
  { BFD_RELOC_32_PLT_PCREL, R_68K_PLT32 },
  { BFD_RELOC_16_PLT_PCREL, R_68K_PLT16 },
  { BFD_RELOC_8_PLT_PCREL, R_68K_PLT8 },
  { BFD_RELOC_32_PLTOFF, R_68K_PLT32O },
  { BFD_RELOC_16_PLTOFF, R_68K_PLT16O },
  { BFD_RELOC_8_PLTOFF, R_68K_PLT8O },
  { BFD_RELOC_NONE, R_68K_COPY },
  { BFD_RELOC_68K_GLOB_DAT, R_68K_GLOB_DAT },
  { BFD_RELOC_68K_JMP_SLOT, R_68K_JMP_SLOT },
  { BFD_RELOC_68K_RELATIVE, R_68K_RELATIVE },
  { BFD_RELOC_CTOR, R_68K_32 },
  { BFD_RELOC_VTABLE_INHERIT, R_68K_GNU_VTINHERIT },
  { BFD_RELOC_VTABLE_ENTRY, R_68K_GNU_VTENTRY },
  { BFD_RELOC_68K_TLS_GD32, R_68K_TLS_GD32 },
  { BFD_RELOC_68K_TLS_GD16, R_68K_TLS_GD16 },
  { BFD_RELOC_68K_TLS_GD8, R_68K_TLS_GD8 },
  { BFD_RELOC_68K_TLS_LDM32, R_68K_TLS_LDM32 },
  { BFD_RELOC_68K_TLS_LDM16, R_68K_TLS_LDM16 },
  { BFD_RELOC_68K_TLS_LDM8, R_68K_TLS_LDM8 },
  { BFD_RELOC_68K_TLS_LDO32, R_68K_TLS_LDO32 },
  { BFD_RELOC_68K_TLS_LDO16, R_68K_TLS_LDO16 },
  { BFD_RELOC_68K_TLS_LDO8, R_68K_TLS_LDO8 },
  { BFD_RELOC_68K_TLS_IE32, R_68K_TLS_IE32 },
  { BFD_RELOC_68K_TLS_IE16, R_68K_TLS_IE16 },
  { BFD_RELOC_68K_TLS_IE8, R_68K_TLS_IE8 },
  { BFD_RELOC_68K_TLS_LE32, R_68K_TLS_LE32 },
  { BFD_RELOC_68K_TLS_LE16, R_68K_TLS_LE16 },
  { BFD_RELOC_68K_TLS_LE8, R_68K_TLS_LE8 },
};

static reloc_howto_type *
reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
		   bfd_reloc_code_real_type code)
{
  unsigned int i;
  for (i = 0; i < sizeof (reloc_map) / sizeof (reloc_map[0]); i++)
    {
      if (reloc_map[i].bfd_val == code)
	return &howto_table[reloc_map[i].elf_val];
    }
  return 0;
}

static reloc_howto_type *
reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (howto_table) / sizeof (howto_table[0]); i++)
    if (howto_table[i].name != NULL
	&& strcasecmp (howto_table[i].name, r_name) == 0)
      return &howto_table[i];

  return NULL;
}

#define bfd_elf32_bfd_reloc_type_lookup reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup reloc_name_lookup
#define ELF_ARCH bfd_arch_m68k
#define ELF_TARGET_ID M68K_ELF_DATA

/* Functions for the m68k ELF linker.  */

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/libc.so.1"

/* Describes one of the various PLT styles.  */

struct elf_m68k_plt_info
{
  /* The size of each PLT entry.  */
  bfd_vma size;

  /* The template for the first PLT entry.  */
  const bfd_byte *plt0_entry;

  /* Offsets of fields in PLT0_ENTRY that require R_68K_PC32 relocations.
     The comments by each member indicate the value that the relocation
     is against.  */
  struct {
    unsigned int got4; /* .got + 4 */
    unsigned int got8; /* .got + 8 */
  } plt0_relocs;

  /* The template for a symbol's PLT entry.  */
  const bfd_byte *symbol_entry;

  /* Offsets of fields in SYMBOL_ENTRY that require R_68K_PC32 relocations.
     The comments by each member indicate the value that the relocation
     is against.  */
  struct {
    unsigned int got; /* the symbol's .got.plt entry */
    unsigned int plt; /* .plt */
  } symbol_relocs;

  /* The offset of the resolver stub from the start of SYMBOL_ENTRY.
     The stub starts with "move.l #relocoffset,%d0".  */
  bfd_vma symbol_resolve_entry;
};

/* The size in bytes of an entry in the procedure linkage table.  */

#define PLT_ENTRY_SIZE 20

/* The first entry in a procedure linkage table looks like this.  See
   the SVR4 ABI m68k supplement to see how this works.  */

static const bfd_byte elf_m68k_plt0_entry[PLT_ENTRY_SIZE] =
{
  0x2f, 0x3b, 0x01, 0x70, /* move.l (%pc,addr),-(%sp) */
  0, 0, 0, 2,		  /* + (.got + 4) - . */
  0x4e, 0xfb, 0x01, 0x71, /* jmp ([%pc,addr]) */
  0, 0, 0, 2,		  /* + (.got + 8) - . */
  0, 0, 0, 0		  /* pad out to 20 bytes.  */
};

/* Subsequent entries in a procedure linkage table look like this.  */

static const bfd_byte elf_m68k_plt_entry[PLT_ENTRY_SIZE] =
{
  0x4e, 0xfb, 0x01, 0x71, /* jmp ([%pc,symbol@GOTPC]) */
  0, 0, 0, 2,		  /* + (.got.plt entry) - . */
  0x2f, 0x3c,		  /* move.l #offset,-(%sp) */
  0, 0, 0, 0,		  /* + reloc index */
  0x60, 0xff,		  /* bra.l .plt */
  0, 0, 0, 0		  /* + .plt - . */
};

static const struct elf_m68k_plt_info elf_m68k_plt_info =
{
  PLT_ENTRY_SIZE,
  elf_m68k_plt0_entry, { 4, 12 },
  elf_m68k_plt_entry, { 4, 16 }, 8
};

#define ISAB_PLT_ENTRY_SIZE 24

static const bfd_byte elf_isab_plt0_entry[ISAB_PLT_ENTRY_SIZE] =
{
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* + (.got + 4) - . */
  0x2f, 0x3b, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l),-(%sp) */
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* + (.got + 8) - . */
  0x20, 0x7b, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l), %a0 */
  0x4e, 0xd0,		  /* jmp (%a0) */
  0x4e, 0x71		  /* nop */
};

/* Subsequent entries in a procedure linkage table look like this.  */

static const bfd_byte elf_isab_plt_entry[ISAB_PLT_ENTRY_SIZE] =
{
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* + (.got.plt entry) - . */
  0x20, 0x7b, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l), %a0 */
  0x4e, 0xd0,		  /* jmp (%a0) */
  0x2f, 0x3c,		  /* move.l #offset,-(%sp) */
  0, 0, 0, 0,		  /* + reloc index */
  0x60, 0xff,		  /* bra.l .plt */
  0, 0, 0, 0		  /* + .plt - . */
};

static const struct elf_m68k_plt_info elf_isab_plt_info =
{
  ISAB_PLT_ENTRY_SIZE,
  elf_isab_plt0_entry, { 2, 12 },
  elf_isab_plt_entry, { 2, 20 }, 12
};

#define ISAC_PLT_ENTRY_SIZE 24

static const bfd_byte elf_isac_plt0_entry[ISAC_PLT_ENTRY_SIZE] =
{
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* replaced with .got + 4 - . */
  0x2e, 0xbb, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l),(%sp) */
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* replaced with .got + 8 - . */
  0x20, 0x7b, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l), %a0 */
  0x4e, 0xd0,		  /* jmp (%a0) */
  0x4e, 0x71		  /* nop */
};

/* Subsequent entries in a procedure linkage table look like this.  */

static const bfd_byte elf_isac_plt_entry[ISAC_PLT_ENTRY_SIZE] =
{
  0x20, 0x3c,		  /* move.l #offset,%d0 */
  0, 0, 0, 0,		  /* replaced with (.got entry) - . */
  0x20, 0x7b, 0x08, 0xfa, /* move.l (-6,%pc,%d0:l), %a0 */
  0x4e, 0xd0,		  /* jmp (%a0) */
  0x2f, 0x3c,		  /* move.l #offset,-(%sp) */
  0, 0, 0, 0,		  /* replaced with offset into relocation table */
  0x61, 0xff,		  /* bsr.l .plt */
  0, 0, 0, 0		  /* replaced with .plt - . */
};

static const struct elf_m68k_plt_info elf_isac_plt_info =
{
  ISAC_PLT_ENTRY_SIZE,
  elf_isac_plt0_entry, { 2, 12},
  elf_isac_plt_entry, { 2, 20 }, 12
};

#define CPU32_PLT_ENTRY_SIZE 24
/* Procedure linkage table entries for the cpu32 */
static const bfd_byte elf_cpu32_plt0_entry[CPU32_PLT_ENTRY_SIZE] =
{
  0x2f, 0x3b, 0x01, 0x70, /* move.l (%pc,addr),-(%sp) */
  0, 0, 0, 2,		  /* + (.got + 4) - . */
  0x22, 0x7b, 0x01, 0x70, /* moveal %pc@(0xc), %a1 */
  0, 0, 0, 2,		  /* + (.got + 8) - . */
  0x4e, 0xd1,		  /* jmp %a1@ */
  0, 0, 0, 0,		  /* pad out to 24 bytes.  */
  0, 0
};

static const bfd_byte elf_cpu32_plt_entry[CPU32_PLT_ENTRY_SIZE] =
{
  0x22, 0x7b, 0x01, 0x70,  /* moveal %pc@(0xc), %a1 */
  0, 0, 0, 2,		   /* + (.got.plt entry) - . */
  0x4e, 0xd1,		   /* jmp %a1@ */
  0x2f, 0x3c,		   /* move.l #offset,-(%sp) */
  0, 0, 0, 0,		   /* + reloc index */
  0x60, 0xff,		   /* bra.l .plt */
  0, 0, 0, 0,		   /* + .plt - . */
  0, 0
};

static const struct elf_m68k_plt_info elf_cpu32_plt_info =
{
  CPU32_PLT_ENTRY_SIZE,
  elf_cpu32_plt0_entry, { 4, 12 },
  elf_cpu32_plt_entry, { 4, 18 }, 10
};

/* The m68k linker needs to keep track of the number of relocs that it
   decides to copy in check_relocs for each symbol.  This is so that it
   can discard PC relative relocs if it doesn't need them when linking
   with -Bsymbolic.  We store the information in a field extending the
   regular ELF linker hash table.  */

/* This structure keeps track of the number of PC relative relocs we have
   copied for a given symbol.  */

struct elf_m68k_pcrel_relocs_copied
{
  /* Next section.  */
  struct elf_m68k_pcrel_relocs_copied *next;
  /* A section in dynobj.  */
  asection *section;
  /* Number of relocs copied in this section.  */
  bfd_size_type count;
};

/* Forward declaration.  */
struct elf_m68k_got_entry;

/* m68k ELF linker hash entry.  */

struct elf_m68k_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Number of PC relative relocs copied for this symbol.  */
  struct elf_m68k_pcrel_relocs_copied *pcrel_relocs_copied;

  /* Key to got_entries.  */
  unsigned long got_entry_key;

  /* List of GOT entries for this symbol.  This list is build during
     offset finalization and is used within elf_m68k_finish_dynamic_symbol
     to traverse all GOT entries for a particular symbol.

     ??? We could've used root.got.glist field instead, but having
     a separate field is cleaner.  */
  struct elf_m68k_got_entry *glist;
};

#define elf_m68k_hash_entry(ent) ((struct elf_m68k_link_hash_entry *) (ent))

/* Key part of GOT entry in hashtable.  */
struct elf_m68k_got_entry_key
{
  /* BFD in which this symbol was defined.  NULL for global symbols.  */
  const bfd *bfd;

  /* Symbol index.  Either local symbol index or h->got_entry_key.  */
  unsigned long symndx;

  /* Type is one of R_68K_GOT{8, 16, 32}O, R_68K_TLS_GD{8, 16, 32},
     R_68K_TLS_LDM{8, 16, 32} or R_68K_TLS_IE{8, 16, 32}.

     From perspective of hashtable key, only elf_m68k_got_reloc_type (type)
     matters.  That is, we distinguish between, say, R_68K_GOT16O
     and R_68K_GOT32O when allocating offsets, but they are considered to be
     the same when searching got->entries.  */
  enum elf_m68k_reloc_type type;
};

/* Size of the GOT offset suitable for relocation.  */
enum elf_m68k_got_offset_size { R_8, R_16, R_32, R_LAST };

/* Entry of the GOT.  */
struct elf_m68k_got_entry
{
  /* GOT entries are put into a got->entries hashtable.  This is the key.  */
  struct elf_m68k_got_entry_key key_;

  /* GOT entry data.  We need s1 before offset finalization and s2 after.  */
  union
  {
    struct
    {
      /* Number of times this entry is referenced.  */
      bfd_vma refcount;
    } s1;

    struct
    {
      /* Offset from the start of .got section.  To calculate offset relative
	 to GOT pointer one should subtract got->offset from this value.  */
      bfd_vma offset;

      /* Pointer to the next GOT entry for this global symbol.
	 Symbols have at most one entry in one GOT, but might
	 have entries in more than one GOT.
	 Root of this list is h->glist.
	 NULL for local symbols.  */
      struct elf_m68k_got_entry *next;
    } s2;
  } u;
};

/* Return representative type for relocation R_TYPE.
   This is used to avoid enumerating many relocations in comparisons,
   switches etc.  */

static enum elf_m68k_reloc_type
elf_m68k_reloc_got_type (enum elf_m68k_reloc_type r_type)
{
  switch (r_type)
    {
      /* In most cases R_68K_GOTx relocations require the very same
	 handling as R_68K_GOT32O relocation.  In cases when we need
	 to distinguish between the two, we use explicitly compare against
	 r_type.  */
    case R_68K_GOT32:
    case R_68K_GOT16:
    case R_68K_GOT8:
    case R_68K_GOT32O:
    case R_68K_GOT16O:
    case R_68K_GOT8O:
      return R_68K_GOT32O;

    case R_68K_TLS_GD32:
    case R_68K_TLS_GD16:
    case R_68K_TLS_GD8:
      return R_68K_TLS_GD32;

    case R_68K_TLS_LDM32:
    case R_68K_TLS_LDM16:
    case R_68K_TLS_LDM8:
      return R_68K_TLS_LDM32;

    case R_68K_TLS_IE32:
    case R_68K_TLS_IE16:
    case R_68K_TLS_IE8:
      return R_68K_TLS_IE32;

    default:
      BFD_ASSERT (false);
      return 0;
    }
}

/* Return size of the GOT entry offset for relocation R_TYPE.  */

static enum elf_m68k_got_offset_size
elf_m68k_reloc_got_offset_size (enum elf_m68k_reloc_type r_type)
{
  switch (r_type)
    {
    case R_68K_GOT32: case R_68K_GOT16: case R_68K_GOT8:
    case R_68K_GOT32O: case R_68K_TLS_GD32: case R_68K_TLS_LDM32:
    case R_68K_TLS_IE32:
      return R_32;

    case R_68K_GOT16O: case R_68K_TLS_GD16: case R_68K_TLS_LDM16:
    case R_68K_TLS_IE16:
      return R_16;

    case R_68K_GOT8O: case R_68K_TLS_GD8: case R_68K_TLS_LDM8:
    case R_68K_TLS_IE8:
      return R_8;

    default:
      BFD_ASSERT (false);
      return 0;
    }
}

/* Return number of GOT entries we need to allocate in GOT for
   relocation R_TYPE.  */

static bfd_vma
elf_m68k_reloc_got_n_slots (enum elf_m68k_reloc_type r_type)
{
  switch (elf_m68k_reloc_got_type (r_type))
    {
    case R_68K_GOT32O:
    case R_68K_TLS_IE32:
      return 1;

    case R_68K_TLS_GD32:
    case R_68K_TLS_LDM32:
      return 2;

    default:
      BFD_ASSERT (false);
      return 0;
    }
}

/* Return TRUE if relocation R_TYPE is a TLS one.  */

static bool
elf_m68k_reloc_tls_p (enum elf_m68k_reloc_type r_type)
{
  switch (r_type)
    {
    case R_68K_TLS_GD32: case R_68K_TLS_GD16: case R_68K_TLS_GD8:
    case R_68K_TLS_LDM32: case R_68K_TLS_LDM16: case R_68K_TLS_LDM8:
    case R_68K_TLS_LDO32: case R_68K_TLS_LDO16: case R_68K_TLS_LDO8:
    case R_68K_TLS_IE32: case R_68K_TLS_IE16: case R_68K_TLS_IE8:
    case R_68K_TLS_LE32: case R_68K_TLS_LE16: case R_68K_TLS_LE8:
    case R_68K_TLS_DTPMOD32: case R_68K_TLS_DTPREL32: case R_68K_TLS_TPREL32:
      return true;

    default:
      return false;
    }
}

/* Data structure representing a single GOT.  */
struct elf_m68k_got
{
  /* Hashtable of 'struct elf_m68k_got_entry's.
     Starting size of this table is the maximum number of
     R_68K_GOT8O entries.  */
  htab_t entries;

  /* Number of R_x slots in this GOT.  Some (e.g., TLS) entries require
     several GOT slots.

     n_slots[R_8] is the count of R_8 slots in this GOT.
     n_slots[R_16] is the cumulative count of R_8 and R_16 slots
     in this GOT.
     n_slots[R_32] is the cumulative count of R_8, R_16 and R_32 slots
     in this GOT.  This is the total number of slots.  */
  bfd_vma n_slots[R_LAST];

  /* Number of local (entry->key_.h == NULL) slots in this GOT.
     This is only used to properly calculate size of .rela.got section;
     see elf_m68k_partition_multi_got.  */
  bfd_vma local_n_slots;

  /* Offset of this GOT relative to beginning of .got section.  */
  bfd_vma offset;
};

/* BFD and its GOT.  This is an entry in multi_got->bfd2got hashtable.  */
struct elf_m68k_bfd2got_entry
{
  /* BFD.  */
  const bfd *bfd;

  /* Assigned GOT.  Before partitioning multi-GOT each BFD has its own
     GOT structure.  After partitioning several BFD's might [and often do]
     share a single GOT.  */
  struct elf_m68k_got *got;
};

/* The main data structure holding all the pieces.  */
struct elf_m68k_multi_got
{
  /* Hashtable mapping each BFD to its GOT.  If a BFD doesn't have an entry
     here, then it doesn't need a GOT (this includes the case of a BFD
     having an empty GOT).

     ??? This hashtable can be replaced by an array indexed by bfd->id.  */
  htab_t bfd2got;

  /* Next symndx to assign a global symbol.
     h->got_entry_key is initialized from this counter.  */
  unsigned long global_symndx;
};

/* m68k ELF linker hash table.  */

struct elf_m68k_link_hash_table
{
  struct elf_link_hash_table root;

  /* The PLT format used by this link, or NULL if the format has not
     yet been chosen.  */
  const struct elf_m68k_plt_info *plt_info;

  /* True, if GP is loaded within each function which uses it.
     Set to TRUE when GOT negative offsets or multi-GOT is enabled.  */
  bool local_gp_p;

  /* Switch controlling use of negative offsets to double the size of GOTs.  */
  bool use_neg_got_offsets_p;

  /* Switch controlling generation of multiple GOTs.  */
  bool allow_multigot_p;

  /* Multi-GOT data structure.  */
  struct elf_m68k_multi_got multi_got_;
};

/* Get the m68k ELF linker hash table from a link_info structure.  */

#define elf_m68k_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == M68K_ELF_DATA)		\
   ? (struct elf_m68k_link_hash_table *) (p)->hash : NULL)

/* Shortcut to multi-GOT data.  */
#define elf_m68k_multi_got(INFO) (&elf_m68k_hash_table (INFO)->multi_got_)

/* Create an entry in an m68k ELF linker hash table.  */

static struct bfd_hash_entry *
elf_m68k_link_hash_newfunc (struct bfd_hash_entry *entry,
			    struct bfd_hash_table *table,
			    const char *string)
{
  struct bfd_hash_entry *ret = entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
			     sizeof (struct elf_m68k_link_hash_entry));
  if (ret == NULL)
    return ret;

  /* Call the allocation method of the superclass.  */
  ret = _bfd_elf_link_hash_newfunc (ret, table, string);
  if (ret != NULL)
    {
      elf_m68k_hash_entry (ret)->pcrel_relocs_copied = NULL;
      elf_m68k_hash_entry (ret)->got_entry_key = 0;
      elf_m68k_hash_entry (ret)->glist = NULL;
    }

  return ret;
}

/* Destroy an m68k ELF linker hash table.  */

static void
elf_m68k_link_hash_table_free (bfd *obfd)
{
  struct elf_m68k_link_hash_table *htab;

  htab = (struct elf_m68k_link_hash_table *) obfd->link.hash;

  if (htab->multi_got_.bfd2got != NULL)
    {
      htab_delete (htab->multi_got_.bfd2got);
      htab->multi_got_.bfd2got = NULL;
    }
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an m68k ELF linker hash table.  */

static struct bfd_link_hash_table *
elf_m68k_link_hash_table_create (bfd *abfd)
{
  struct elf_m68k_link_hash_table *ret;
  size_t amt = sizeof (struct elf_m68k_link_hash_table);

  ret = (struct elf_m68k_link_hash_table *) bfd_zmalloc (amt);
  if (ret == (struct elf_m68k_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      elf_m68k_link_hash_newfunc,
				      sizeof (struct elf_m68k_link_hash_entry),
				      M68K_ELF_DATA))
    {
      free (ret);
      return NULL;
    }
  ret->root.root.hash_table_free = elf_m68k_link_hash_table_free;

  ret->multi_got_.global_symndx = 1;

  return &ret->root.root;
}

/* Set the right machine number.  */

static bool
elf32_m68k_object_p (bfd *abfd)
{
  unsigned int mach = 0;
  unsigned features = 0;
  flagword eflags = elf_elfheader (abfd)->e_flags;

  if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_M68000)
    features |= m68000;
  else if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32)
    features |= cpu32;
  else if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO)
    features |= fido_a;
  else
    {
      switch (eflags & EF_M68K_CF_ISA_MASK)
	{
	case EF_M68K_CF_ISA_A_NODIV:
	  features |= mcfisa_a;
	  break;
	case EF_M68K_CF_ISA_A:
	  features |= mcfisa_a|mcfhwdiv;
	  break;
	case EF_M68K_CF_ISA_A_PLUS:
	  features |= mcfisa_a|mcfisa_aa|mcfhwdiv|mcfusp;
	  break;
	case EF_M68K_CF_ISA_B_NOUSP:
	  features |= mcfisa_a|mcfisa_b|mcfhwdiv;
	  break;
	case EF_M68K_CF_ISA_B:
	  features |= mcfisa_a|mcfisa_b|mcfhwdiv|mcfusp;
	  break;
	case EF_M68K_CF_ISA_C:
	  features |= mcfisa_a|mcfisa_c|mcfhwdiv|mcfusp;
	  break;
	case EF_M68K_CF_ISA_C_NODIV:
	  features |= mcfisa_a|mcfisa_c|mcfusp;
	  break;
	}
      switch (eflags & EF_M68K_CF_MAC_MASK)
	{
	case EF_M68K_CF_MAC:
	  features |= mcfmac;
	  break;
	case EF_M68K_CF_EMAC:
	  features |= mcfemac;
	  break;
	}
      if (eflags & EF_M68K_CF_FLOAT)
	features |= cfloat;
    }

  mach = bfd_m68k_features_to_mach (features);
  bfd_default_set_arch_mach (abfd, bfd_arch_m68k, mach);

  return true;
}

/* Somewhat reverse of elf32_m68k_object_p, this sets the e_flag
   field based on the machine number.  */

static bool
elf_m68k_final_write_processing (bfd *abfd)
{
  int mach = bfd_get_mach (abfd);
  unsigned long e_flags = elf_elfheader (abfd)->e_flags;

  if (!e_flags)
    {
      unsigned int arch_mask;

      arch_mask = bfd_m68k_mach_to_features (mach);

      if (arch_mask & m68000)
	e_flags = EF_M68K_M68000;
      else if (arch_mask & cpu32)
	e_flags = EF_M68K_CPU32;
      else if (arch_mask & fido_a)
	e_flags = EF_M68K_FIDO;
      else
	{
	  switch (arch_mask
		  & (mcfisa_a | mcfisa_aa | mcfisa_b | mcfisa_c | mcfhwdiv | mcfusp))
	    {
	    case mcfisa_a:
	      e_flags |= EF_M68K_CF_ISA_A_NODIV;
	      break;
	    case mcfisa_a | mcfhwdiv:
	      e_flags |= EF_M68K_CF_ISA_A;
	      break;
	    case mcfisa_a | mcfisa_aa | mcfhwdiv | mcfusp:
	      e_flags |= EF_M68K_CF_ISA_A_PLUS;
	      break;
	    case mcfisa_a | mcfisa_b | mcfhwdiv:
	      e_flags |= EF_M68K_CF_ISA_B_NOUSP;
	      break;
	    case mcfisa_a | mcfisa_b | mcfhwdiv | mcfusp:
	      e_flags |= EF_M68K_CF_ISA_B;
	      break;
	    case mcfisa_a | mcfisa_c | mcfhwdiv | mcfusp:
	      e_flags |= EF_M68K_CF_ISA_C;
	      break;
	    case mcfisa_a | mcfisa_c | mcfusp:
	      e_flags |= EF_M68K_CF_ISA_C_NODIV;
	      break;
	    }
	  if (arch_mask & mcfmac)
	    e_flags |= EF_M68K_CF_MAC;
	  else if (arch_mask & mcfemac)
	    e_flags |= EF_M68K_CF_EMAC;
	  if (arch_mask & cfloat)
	    e_flags |= EF_M68K_CF_FLOAT | EF_M68K_CFV4E;
	}
      elf_elfheader (abfd)->e_flags = e_flags;
    }
  return _bfd_elf_final_write_processing (abfd);
}

/* Keep m68k-specific flags in the ELF header.  */

static bool
elf32_m68k_set_private_flags (bfd *abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Merge object attributes from IBFD into OBFD.  Warn if
   there are conflicting attributes. */
static bool
m68k_elf_merge_obj_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr, *in_attrs;
  obj_attribute *out_attr, *out_attrs;
  bool ret = true;

  in_attrs = elf_known_obj_attributes (ibfd)[OBJ_ATTR_GNU];
  out_attrs = elf_known_obj_attributes (obfd)[OBJ_ATTR_GNU];

  in_attr = &in_attrs[Tag_GNU_M68K_ABI_FP];
  out_attr = &out_attrs[Tag_GNU_M68K_ABI_FP];

  if (in_attr->i != out_attr->i)
    {
      int in_fp = in_attr->i & 3;
      int out_fp = out_attr->i & 3;
      static bfd *last_fp;

      if (in_fp == 0)
	;
      else if (out_fp == 0)
	{
	  out_attr->type = ATTR_TYPE_FLAG_INT_VAL;
	  out_attr->i ^= in_fp;
	  last_fp = ibfd;
	}
      else if (out_fp == 1 && in_fp == 2)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB uses hard float, %pB uses soft float"),
	     last_fp, ibfd);
	  ret = false;
	}
      else if (out_fp == 2 && in_fp == 1)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB uses hard float, %pB uses soft float"),
	     ibfd, last_fp);
	  ret = false;
	}
    }

  if (!ret)
    {
      out_attr->type = ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_ERROR;
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  return _bfd_elf_merge_object_attributes (ibfd, info);
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */
static bool
elf32_m68k_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword out_flags;
  flagword in_flags;
  flagword out_isa;
  flagword in_isa;
  const bfd_arch_info_type *arch_info;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    /* PR 24523: For non-ELF files do not try to merge any private
       data, but also do not prevent the link from succeeding.  */
    return true;

  /* Get the merged machine.  This checks for incompatibility between
     Coldfire & non-Coldfire flags, incompability between different
     Coldfire ISAs, and incompability between different MAC types.  */
  arch_info = bfd_arch_get_compatible (ibfd, obfd, false);
  if (!arch_info)
    return false;

  bfd_set_arch_mach (obfd, bfd_arch_m68k, arch_info->mach);

  if (!m68k_elf_merge_obj_attributes (ibfd, info))
    return false;

  in_flags = elf_elfheader (ibfd)->e_flags;
  if (!elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      out_flags = in_flags;
    }
  else
    {
      out_flags = elf_elfheader (obfd)->e_flags;
      unsigned int variant_mask;

      if ((in_flags & EF_M68K_ARCH_MASK) == EF_M68K_M68000)
	variant_mask = 0;
      else if ((in_flags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32)
	variant_mask = 0;
      else if ((in_flags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO)
	variant_mask = 0;
      else
	variant_mask = EF_M68K_CF_ISA_MASK;

      in_isa = (in_flags & variant_mask);
      out_isa = (out_flags & variant_mask);
      if (in_isa > out_isa)
	out_flags ^= in_isa ^ out_isa;
      if (((in_flags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32
	   && (out_flags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO)
	  || ((in_flags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO
	      && (out_flags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32))
	out_flags = EF_M68K_FIDO;
      else
      out_flags |= in_flags ^ in_isa;
    }
  elf_elfheader (obfd)->e_flags = out_flags;

  return true;
}

/* Display the flags field.  */

static bool
elf32_m68k_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;
  flagword eflags = elf_elfheader (abfd)->e_flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* Ignore init flag - it may not be set, despite the flags field containing valid data.  */

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_M68000)
    fprintf (file, " [m68000]");
  else if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_CPU32)
    fprintf (file, " [cpu32]");
  else if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_FIDO)
    fprintf (file, " [fido]");
  else
    {
      if ((eflags & EF_M68K_ARCH_MASK) == EF_M68K_CFV4E)
	fprintf (file, " [cfv4e]");

      if (eflags & EF_M68K_CF_ISA_MASK)
	{
	  char const *isa = _("unknown");
	  char const *mac = _("unknown");
	  char const *additional = "";

	  switch (eflags & EF_M68K_CF_ISA_MASK)
	    {
	    case EF_M68K_CF_ISA_A_NODIV:
	      isa = "A";
	      additional = " [nodiv]";
	      break;
	    case EF_M68K_CF_ISA_A:
	      isa = "A";
	      break;
	    case EF_M68K_CF_ISA_A_PLUS:
	      isa = "A+";
	      break;
	    case EF_M68K_CF_ISA_B_NOUSP:
	      isa = "B";
	      additional = " [nousp]";
	      break;
	    case EF_M68K_CF_ISA_B:
	      isa = "B";
	      break;
	    case EF_M68K_CF_ISA_C:
	      isa = "C";
	      break;
	    case EF_M68K_CF_ISA_C_NODIV:
	      isa = "C";
	      additional = " [nodiv]";
	      break;
	    }
	  fprintf (file, " [isa %s]%s", isa, additional);

	  if (eflags & EF_M68K_CF_FLOAT)
	    fprintf (file, " [float]");

	  switch (eflags & EF_M68K_CF_MAC_MASK)
	    {
	    case 0:
	      mac = NULL;
	      break;
	    case EF_M68K_CF_MAC:
	      mac = "mac";
	      break;
	    case EF_M68K_CF_EMAC:
	      mac = "emac";
	      break;
	    case EF_M68K_CF_EMAC_B:
	      mac = "emac_b";
	      break;
	    }
	  if (mac)
	    fprintf (file, " [%s]", mac);
	}
    }

  fputc ('\n', file);

  return true;
}

/* Multi-GOT support implementation design:

   Multi-GOT starts in check_relocs hook.  There we scan all
   relocations of a BFD and build a local GOT (struct elf_m68k_got)
   for it.  If a single BFD appears to require too many GOT slots with
   R_68K_GOT8O or R_68K_GOT16O relocations, we fail with notification
   to user.
   After check_relocs has been invoked for each input BFD, we have
   constructed a GOT for each input BFD.

   To minimize total number of GOTs required for a particular output BFD
   (as some environments support only 1 GOT per output object) we try
   to merge some of the GOTs to share an offset space.  Ideally [and in most
   cases] we end up with a single GOT.  In cases when there are too many
   restricted relocations (e.g., R_68K_GOT16O relocations) we end up with
   several GOTs, assuming the environment can handle them.

   Partitioning is done in elf_m68k_partition_multi_got.  We start with
   an empty GOT and traverse bfd2got hashtable putting got_entries from
   local GOTs to the new 'big' one.  We do that by constructing an
   intermediate GOT holding all the entries the local GOT has and the big
   GOT lacks.  Then we check if there is room in the big GOT to accomodate
   all the entries from diff.  On success we add those entries to the big
   GOT; on failure we start the new 'big' GOT and retry the adding of
   entries from the local GOT.  Note that this retry will always succeed as
   each local GOT doesn't overflow the limits.  After partitioning we
   end up with each bfd assigned one of the big GOTs.  GOT entries in the
   big GOTs are initialized with GOT offsets.  Note that big GOTs are
   positioned consequently in program space and represent a single huge GOT
   to the outside world.

   After that we get to elf_m68k_relocate_section.  There we
   adjust relocations of GOT pointer (_GLOBAL_OFFSET_TABLE_) and symbol
   relocations to refer to appropriate [assigned to current input_bfd]
   big GOT.

   Notes:

   GOT entry type: We have several types of GOT entries.
   * R_8 type is used in entries for symbols that have at least one
   R_68K_GOT8O or R_68K_TLS_*8 relocation.  We can have at most 0x40
   such entries in one GOT.
   * R_16 type is used in entries for symbols that have at least one
   R_68K_GOT16O or R_68K_TLS_*16 relocation and no R_8 relocations.
   We can have at most 0x4000 such entries in one GOT.
   * R_32 type is used in all other cases.  We can have as many
   such entries in one GOT as we'd like.
   When counting relocations we have to include the count of the smaller
   ranged relocations in the counts of the larger ranged ones in order
   to correctly detect overflow.

   Sorting the GOT: In each GOT starting offsets are assigned to
   R_8 entries, which are followed by R_16 entries, and
   R_32 entries go at the end.  See finalize_got_offsets for details.

   Negative GOT offsets: To double usable offset range of GOTs we use
   negative offsets.  As we assign entries with GOT offsets relative to
   start of .got section, the offset values are positive.  They become
   negative only in relocate_section where got->offset value is
   subtracted from them.

   3 special GOT entries: There are 3 special GOT entries used internally
   by loader.  These entries happen to be placed to .got.plt section,
   so we don't do anything about them in multi-GOT support.

   Memory management: All data except for hashtables
   multi_got->bfd2got and got->entries are allocated on
   elf_hash_table (info)->dynobj bfd (for this reason we pass 'info'
   to most functions), so we don't need to care to free them.  At the
   moment of allocation hashtables are being linked into main data
   structure (multi_got), all pieces of which are reachable from
   elf_m68k_multi_got (info).  We deallocate them in
   elf_m68k_link_hash_table_free.  */

/* Initialize GOT.  */

static void
elf_m68k_init_got (struct elf_m68k_got *got)
{
  got->entries = NULL;
  got->n_slots[R_8] = 0;
  got->n_slots[R_16] = 0;
  got->n_slots[R_32] = 0;
  got->local_n_slots = 0;
  got->offset = (bfd_vma) -1;
}

/* Destruct GOT.  */

static void
elf_m68k_clear_got (struct elf_m68k_got *got)
{
  if (got->entries != NULL)
    {
      htab_delete (got->entries);
      got->entries = NULL;
    }
}

/* Create and empty GOT structure.  INFO is the context where memory
   should be allocated.  */

static struct elf_m68k_got *
elf_m68k_create_empty_got (struct bfd_link_info *info)
{
  struct elf_m68k_got *got;

  got = bfd_alloc (elf_hash_table (info)->dynobj, sizeof (*got));
  if (got == NULL)
    return NULL;

  elf_m68k_init_got (got);

  return got;
}

/* Initialize KEY.  */

static void
elf_m68k_init_got_entry_key (struct elf_m68k_got_entry_key *key,
			     struct elf_link_hash_entry *h,
			     const bfd *abfd, unsigned long symndx,
			     enum elf_m68k_reloc_type reloc_type)
{
  if (elf_m68k_reloc_got_type (reloc_type) == R_68K_TLS_LDM32)
    /* All TLS_LDM relocations share a single GOT entry.  */
    {
      key->bfd = NULL;
      key->symndx = 0;
    }
  else if (h != NULL)
    /* Global symbols are identified with their got_entry_key.  */
    {
      key->bfd = NULL;
      key->symndx = elf_m68k_hash_entry (h)->got_entry_key;
      BFD_ASSERT (key->symndx != 0);
    }
  else
    /* Local symbols are identified by BFD they appear in and symndx.  */
    {
      key->bfd = abfd;
      key->symndx = symndx;
    }

  key->type = reloc_type;
}

/* Calculate hash of got_entry.
   ??? Is it good?  */

static hashval_t
elf_m68k_got_entry_hash (const void *_entry)
{
  const struct elf_m68k_got_entry_key *key;

  key = &((const struct elf_m68k_got_entry *) _entry)->key_;

  return (key->symndx
	  + (key->bfd != NULL ? (int) key->bfd->id : -1)
	  + elf_m68k_reloc_got_type (key->type));
}

/* Check if two got entries are equal.  */

static int
elf_m68k_got_entry_eq (const void *_entry1, const void *_entry2)
{
  const struct elf_m68k_got_entry_key *key1;
  const struct elf_m68k_got_entry_key *key2;

  key1 = &((const struct elf_m68k_got_entry *) _entry1)->key_;
  key2 = &((const struct elf_m68k_got_entry *) _entry2)->key_;

  return (key1->bfd == key2->bfd
	  && key1->symndx == key2->symndx
	  && (elf_m68k_reloc_got_type (key1->type)
	      == elf_m68k_reloc_got_type (key2->type)));
}

/* When using negative offsets, we allocate one extra R_8, one extra R_16
   and one extra R_32 slots to simplify handling of 2-slot entries during
   offset allocation -- hence -1 for R_8 slots and -2 for R_16 slots.  */

/* Maximal number of R_8 slots in a single GOT.  */
#define ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT(INFO)		\
  (elf_m68k_hash_table (INFO)->use_neg_got_offsets_p		\
   ? (0x40 - 1)							\
   : 0x20)

/* Maximal number of R_8 and R_16 slots in a single GOT.  */
#define ELF_M68K_R_8_16_MAX_N_SLOTS_IN_GOT(INFO)		\
  (elf_m68k_hash_table (INFO)->use_neg_got_offsets_p		\
   ? (0x4000 - 2)						\
   : 0x2000)

/* SEARCH - simply search the hashtable, don't insert new entries or fail when
   the entry cannot be found.
   FIND_OR_CREATE - search for an existing entry, but create new if there's
   no such.
   MUST_FIND - search for an existing entry and assert that it exist.
   MUST_CREATE - assert that there's no such entry and create new one.  */
enum elf_m68k_get_entry_howto
  {
    SEARCH,
    FIND_OR_CREATE,
    MUST_FIND,
    MUST_CREATE
  };

/* Get or create (depending on HOWTO) entry with KEY in GOT.
   INFO is context in which memory should be allocated (can be NULL if
   HOWTO is SEARCH or MUST_FIND).  */

static struct elf_m68k_got_entry *
elf_m68k_get_got_entry (struct elf_m68k_got *got,
			const struct elf_m68k_got_entry_key *key,
			enum elf_m68k_get_entry_howto howto,
			struct bfd_link_info *info)
{
  struct elf_m68k_got_entry entry_;
  struct elf_m68k_got_entry *entry;
  void **ptr;

  BFD_ASSERT ((info == NULL) == (howto == SEARCH || howto == MUST_FIND));

  if (got->entries == NULL)
    /* This is the first entry in ABFD.  Initialize hashtable.  */
    {
      if (howto == SEARCH)
	return NULL;

      got->entries = htab_try_create (ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT
				      (info),
				      elf_m68k_got_entry_hash,
				      elf_m68k_got_entry_eq, NULL);
      if (got->entries == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  return NULL;
	}
    }

  entry_.key_ = *key;
  ptr = htab_find_slot (got->entries, &entry_,
			(howto == SEARCH || howto == MUST_FIND ? NO_INSERT
			 : INSERT));
  if (ptr == NULL)
    {
      if (howto == SEARCH)
	/* Entry not found.  */
	return NULL;

      if (howto == MUST_FIND)
	abort ();

      /* We're out of memory.  */
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  if (*ptr == NULL)
    /* We didn't find the entry and we're asked to create a new one.  */
    {
      if (howto == MUST_FIND)
	abort ();

      BFD_ASSERT (howto != SEARCH);

      entry = bfd_alloc (elf_hash_table (info)->dynobj, sizeof (*entry));
      if (entry == NULL)
	return NULL;

      /* Initialize new entry.  */
      entry->key_ = *key;

      entry->u.s1.refcount = 0;

      /* Mark the entry as not initialized.  */
      entry->key_.type = R_68K_max;

      *ptr = entry;
    }
  else
    /* We found the entry.  */
    {
      BFD_ASSERT (howto != MUST_CREATE);

      entry = *ptr;
    }

  return entry;
}

/* Update GOT counters when merging entry of WAS type with entry of NEW type.
   Return the value to which ENTRY's type should be set.  */

static enum elf_m68k_reloc_type
elf_m68k_update_got_entry_type (struct elf_m68k_got *got,
				enum elf_m68k_reloc_type was,
				enum elf_m68k_reloc_type new_reloc)
{
  enum elf_m68k_got_offset_size was_size;
  enum elf_m68k_got_offset_size new_size;
  bfd_vma n_slots;

  if (was == R_68K_max)
    /* The type of the entry is not initialized yet.  */
    {
      /* Update all got->n_slots counters, including n_slots[R_32].  */
      was_size = R_LAST;

      was = new_reloc;
    }
  else
    {
      /* !!! We, probably, should emit an error rather then fail on assert
	 in such a case.  */
      BFD_ASSERT (elf_m68k_reloc_got_type (was)
		  == elf_m68k_reloc_got_type (new_reloc));

      was_size = elf_m68k_reloc_got_offset_size (was);
    }

  new_size = elf_m68k_reloc_got_offset_size (new_reloc);
  n_slots = elf_m68k_reloc_got_n_slots (new_reloc);

  while (was_size > new_size)
    {
      --was_size;
      got->n_slots[was_size] += n_slots;
    }

  if (new_reloc > was)
    /* Relocations are ordered from bigger got offset size to lesser,
       so choose the relocation type with lesser offset size.  */
    was = new_reloc;

  return was;
}

/* Add new or update existing entry to GOT.
   H, ABFD, TYPE and SYMNDX is data for the entry.
   INFO is a context where memory should be allocated.  */

static struct elf_m68k_got_entry *
elf_m68k_add_entry_to_got (struct elf_m68k_got *got,
			   struct elf_link_hash_entry *h,
			   const bfd *abfd,
			   enum elf_m68k_reloc_type reloc_type,
			   unsigned long symndx,
			   struct bfd_link_info *info)
{
  struct elf_m68k_got_entry_key key_;
  struct elf_m68k_got_entry *entry;

  if (h != NULL && elf_m68k_hash_entry (h)->got_entry_key == 0)
    elf_m68k_hash_entry (h)->got_entry_key
      = elf_m68k_multi_got (info)->global_symndx++;

  elf_m68k_init_got_entry_key (&key_, h, abfd, symndx, reloc_type);

  entry = elf_m68k_get_got_entry (got, &key_, FIND_OR_CREATE, info);
  if (entry == NULL)
    return NULL;

  /* Determine entry's type and update got->n_slots counters.  */
  entry->key_.type = elf_m68k_update_got_entry_type (got,
						     entry->key_.type,
						     reloc_type);

  /* Update refcount.  */
  ++entry->u.s1.refcount;

  if (entry->u.s1.refcount == 1)
    /* We see this entry for the first time.  */
    {
      if (entry->key_.bfd != NULL)
	got->local_n_slots += elf_m68k_reloc_got_n_slots (entry->key_.type);
    }

  BFD_ASSERT (got->n_slots[R_32] >= got->local_n_slots);

  if ((got->n_slots[R_8]
       > ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT (info))
      || (got->n_slots[R_16]
	  > ELF_M68K_R_8_16_MAX_N_SLOTS_IN_GOT (info)))
    /* This BFD has too many relocation.  */
    {
      if (got->n_slots[R_8] > ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT (info))
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB: GOT overflow: "
			      "number of relocations with 8-bit "
			      "offset > %d"),
			    abfd,
			    ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT (info));
      else
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB: GOT overflow: "
			      "number of relocations with 8- or 16-bit "
			      "offset > %d"),
			    abfd,
			    ELF_M68K_R_8_16_MAX_N_SLOTS_IN_GOT (info));

      return NULL;
    }

  return entry;
}

/* Compute the hash value of the bfd in a bfd2got hash entry.  */

static hashval_t
elf_m68k_bfd2got_entry_hash (const void *entry)
{
  const struct elf_m68k_bfd2got_entry *e;

  e = (const struct elf_m68k_bfd2got_entry *) entry;

  return e->bfd->id;
}

/* Check whether two hash entries have the same bfd.  */

static int
elf_m68k_bfd2got_entry_eq (const void *entry1, const void *entry2)
{
  const struct elf_m68k_bfd2got_entry *e1;
  const struct elf_m68k_bfd2got_entry *e2;

  e1 = (const struct elf_m68k_bfd2got_entry *) entry1;
  e2 = (const struct elf_m68k_bfd2got_entry *) entry2;

  return e1->bfd == e2->bfd;
}

/* Destruct a bfd2got entry.  */

static void
elf_m68k_bfd2got_entry_del (void *_entry)
{
  struct elf_m68k_bfd2got_entry *entry;

  entry = (struct elf_m68k_bfd2got_entry *) _entry;

  BFD_ASSERT (entry->got != NULL);
  elf_m68k_clear_got (entry->got);
}

/* Find existing or create new (depending on HOWTO) bfd2got entry in
   MULTI_GOT.  ABFD is the bfd we need a GOT for.  INFO is a context where
   memory should be allocated.  */

static struct elf_m68k_bfd2got_entry *
elf_m68k_get_bfd2got_entry (struct elf_m68k_multi_got *multi_got,
			    const bfd *abfd,
			    enum elf_m68k_get_entry_howto howto,
			    struct bfd_link_info *info)
{
  struct elf_m68k_bfd2got_entry entry_;
  void **ptr;
  struct elf_m68k_bfd2got_entry *entry;

  BFD_ASSERT ((info == NULL) == (howto == SEARCH || howto == MUST_FIND));

  if (multi_got->bfd2got == NULL)
    /* This is the first GOT.  Initialize bfd2got.  */
    {
      if (howto == SEARCH)
	return NULL;

      multi_got->bfd2got = htab_try_create (1, elf_m68k_bfd2got_entry_hash,
					    elf_m68k_bfd2got_entry_eq,
					    elf_m68k_bfd2got_entry_del);
      if (multi_got->bfd2got == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  return NULL;
	}
    }

  entry_.bfd = abfd;
  ptr = htab_find_slot (multi_got->bfd2got, &entry_,
			(howto == SEARCH || howto == MUST_FIND ? NO_INSERT
			 : INSERT));
  if (ptr == NULL)
    {
      if (howto == SEARCH)
	/* Entry not found.  */
	return NULL;

      if (howto == MUST_FIND)
	abort ();

      /* We're out of memory.  */
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  if (*ptr == NULL)
    /* Entry was not found.  Create new one.  */
    {
      if (howto == MUST_FIND)
	abort ();

      BFD_ASSERT (howto != SEARCH);

      entry = ((struct elf_m68k_bfd2got_entry *)
	       bfd_alloc (elf_hash_table (info)->dynobj, sizeof (*entry)));
      if (entry == NULL)
	return NULL;

      entry->bfd = abfd;

      entry->got = elf_m68k_create_empty_got (info);
      if (entry->got == NULL)
	return NULL;

      *ptr = entry;
    }
  else
    {
      BFD_ASSERT (howto != MUST_CREATE);

      /* Return existing entry.  */
      entry = *ptr;
    }

  return entry;
}

struct elf_m68k_can_merge_gots_arg
{
  /* A current_got that we constructing a DIFF against.  */
  struct elf_m68k_got *big;

  /* GOT holding entries not present or that should be changed in
     BIG.  */
  struct elf_m68k_got *diff;

  /* Context where to allocate memory.  */
  struct bfd_link_info *info;

  /* Error flag.  */
  bool error_p;
};

/* Process a single entry from the small GOT to see if it should be added
   or updated in the big GOT.  */

static int
elf_m68k_can_merge_gots_1 (void **_entry_ptr, void *_arg)
{
  const struct elf_m68k_got_entry *entry1;
  struct elf_m68k_can_merge_gots_arg *arg;
  const struct elf_m68k_got_entry *entry2;
  enum elf_m68k_reloc_type type;

  entry1 = (const struct elf_m68k_got_entry *) *_entry_ptr;
  arg = (struct elf_m68k_can_merge_gots_arg *) _arg;

  entry2 = elf_m68k_get_got_entry (arg->big, &entry1->key_, SEARCH, NULL);

  if (entry2 != NULL)
    /* We found an existing entry.  Check if we should update it.  */
    {
      type = elf_m68k_update_got_entry_type (arg->diff,
					     entry2->key_.type,
					     entry1->key_.type);

      if (type == entry2->key_.type)
	/* ENTRY1 doesn't update data in ENTRY2.  Skip it.
	   To skip creation of difference entry we use the type,
	   which we won't see in GOT entries for sure.  */
	type = R_68K_max;
    }
  else
    /* We didn't find the entry.  Add entry1 to DIFF.  */
    {
      BFD_ASSERT (entry1->key_.type != R_68K_max);

      type = elf_m68k_update_got_entry_type (arg->diff,
					     R_68K_max, entry1->key_.type);

      if (entry1->key_.bfd != NULL)
	arg->diff->local_n_slots += elf_m68k_reloc_got_n_slots (type);
    }

  if (type != R_68K_max)
    /* Create an entry in DIFF.  */
    {
      struct elf_m68k_got_entry *entry;

      entry = elf_m68k_get_got_entry (arg->diff, &entry1->key_, MUST_CREATE,
				      arg->info);
      if (entry == NULL)
	{
	  arg->error_p = true;
	  return 0;
	}

      entry->key_.type = type;
    }

  return 1;
}

/* Return TRUE if SMALL GOT can be added to BIG GOT without overflowing it.
   Construct DIFF GOT holding the entries which should be added or updated
   in BIG GOT to accumulate information from SMALL.
   INFO is the context where memory should be allocated.  */

static bool
elf_m68k_can_merge_gots (struct elf_m68k_got *big,
			 const struct elf_m68k_got *small,
			 struct bfd_link_info *info,
			 struct elf_m68k_got *diff)
{
  struct elf_m68k_can_merge_gots_arg arg_;

  BFD_ASSERT (small->offset == (bfd_vma) -1);

  arg_.big = big;
  arg_.diff = diff;
  arg_.info = info;
  arg_.error_p = false;
  htab_traverse_noresize (small->entries, elf_m68k_can_merge_gots_1, &arg_);
  if (arg_.error_p)
    {
      diff->offset = 0;
      return false;
    }

  /* Check for overflow.  */
  if ((big->n_slots[R_8] + arg_.diff->n_slots[R_8]
       > ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT (info))
      || (big->n_slots[R_16] + arg_.diff->n_slots[R_16]
	  > ELF_M68K_R_8_16_MAX_N_SLOTS_IN_GOT (info)))
    return false;

  return true;
}

struct elf_m68k_merge_gots_arg
{
  /* The BIG got.  */
  struct elf_m68k_got *big;

  /* Context where memory should be allocated.  */
  struct bfd_link_info *info;

  /* Error flag.  */
  bool error_p;
};

/* Process a single entry from DIFF got.  Add or update corresponding
   entry in the BIG got.  */

static int
elf_m68k_merge_gots_1 (void **entry_ptr, void *_arg)
{
  const struct elf_m68k_got_entry *from;
  struct elf_m68k_merge_gots_arg *arg;
  struct elf_m68k_got_entry *to;

  from = (const struct elf_m68k_got_entry *) *entry_ptr;
  arg = (struct elf_m68k_merge_gots_arg *) _arg;

  to = elf_m68k_get_got_entry (arg->big, &from->key_, FIND_OR_CREATE,
			       arg->info);
  if (to == NULL)
    {
      arg->error_p = true;
      return 0;
    }

  BFD_ASSERT (to->u.s1.refcount == 0);
  /* All we need to merge is TYPE.  */
  to->key_.type = from->key_.type;

  return 1;
}

/* Merge data from DIFF to BIG.  INFO is context where memory should be
   allocated.  */

static bool
elf_m68k_merge_gots (struct elf_m68k_got *big,
		     struct elf_m68k_got *diff,
		     struct bfd_link_info *info)
{
  if (diff->entries != NULL)
    /* DIFF is not empty.  Merge it into BIG GOT.  */
    {
      struct elf_m68k_merge_gots_arg arg_;

      /* Merge entries.  */
      arg_.big = big;
      arg_.info = info;
      arg_.error_p = false;
      htab_traverse_noresize (diff->entries, elf_m68k_merge_gots_1, &arg_);
      if (arg_.error_p)
	return false;

      /* Merge counters.  */
      big->n_slots[R_8] += diff->n_slots[R_8];
      big->n_slots[R_16] += diff->n_slots[R_16];
      big->n_slots[R_32] += diff->n_slots[R_32];
      big->local_n_slots += diff->local_n_slots;
    }
  else
    /* DIFF is empty.  */
    {
      BFD_ASSERT (diff->n_slots[R_8] == 0);
      BFD_ASSERT (diff->n_slots[R_16] == 0);
      BFD_ASSERT (diff->n_slots[R_32] == 0);
      BFD_ASSERT (diff->local_n_slots == 0);
    }

  BFD_ASSERT (!elf_m68k_hash_table (info)->allow_multigot_p
	      || ((big->n_slots[R_8]
		   <= ELF_M68K_R_8_MAX_N_SLOTS_IN_GOT (info))
		  && (big->n_slots[R_16]
		      <= ELF_M68K_R_8_16_MAX_N_SLOTS_IN_GOT (info))));

  return true;
}

struct elf_m68k_finalize_got_offsets_arg
{
  /* Ranges of the offsets for GOT entries.
     R_x entries receive offsets between offset1[R_x] and offset2[R_x].
     R_x is R_8, R_16 and R_32.  */
  bfd_vma *offset1;
  bfd_vma *offset2;

  /* Mapping from global symndx to global symbols.
     This is used to build lists of got entries for global symbols.  */
  struct elf_m68k_link_hash_entry **symndx2h;

  bfd_vma n_ldm_entries;
};

/* Assign ENTRY an offset.  Build list of GOT entries for global symbols
   along the way.  */

static int
elf_m68k_finalize_got_offsets_1 (void **entry_ptr, void *_arg)
{
  struct elf_m68k_got_entry *entry;
  struct elf_m68k_finalize_got_offsets_arg *arg;

  enum elf_m68k_got_offset_size got_offset_size;
  bfd_vma entry_size;

  entry = (struct elf_m68k_got_entry *) *entry_ptr;
  arg = (struct elf_m68k_finalize_got_offsets_arg *) _arg;

  /* This should be a fresh entry created in elf_m68k_can_merge_gots.  */
  BFD_ASSERT (entry->u.s1.refcount == 0);

  /* Get GOT offset size for the entry .  */
  got_offset_size = elf_m68k_reloc_got_offset_size (entry->key_.type);

  /* Calculate entry size in bytes.  */
  entry_size = 4 * elf_m68k_reloc_got_n_slots (entry->key_.type);

  /* Check if we should switch to negative range of the offsets. */
  if (arg->offset1[got_offset_size] + entry_size
      > arg->offset2[got_offset_size])
    {
      /* Verify that this is the only switch to negative range for
	 got_offset_size.  If this assertion fails, then we've miscalculated
	 range for got_offset_size entries in
	 elf_m68k_finalize_got_offsets.  */
      BFD_ASSERT (arg->offset2[got_offset_size]
		  != arg->offset2[-(int) got_offset_size - 1]);

      /* Switch.  */
      arg->offset1[got_offset_size] = arg->offset1[-(int) got_offset_size - 1];
      arg->offset2[got_offset_size] = arg->offset2[-(int) got_offset_size - 1];

      /* Verify that now we have enough room for the entry.  */
      BFD_ASSERT (arg->offset1[got_offset_size] + entry_size
		  <= arg->offset2[got_offset_size]);
    }

  /* Assign offset to entry.  */
  entry->u.s2.offset = arg->offset1[got_offset_size];
  arg->offset1[got_offset_size] += entry_size;

  if (entry->key_.bfd == NULL)
    /* Hook up this entry into the list of got_entries of H.  */
    {
      struct elf_m68k_link_hash_entry *h;

      h = arg->symndx2h[entry->key_.symndx];
      if (h != NULL)
	{
	  entry->u.s2.next = h->glist;
	  h->glist = entry;
	}
      else
	/* This should be the entry for TLS_LDM relocation then.  */
	{
	  BFD_ASSERT ((elf_m68k_reloc_got_type (entry->key_.type)
		       == R_68K_TLS_LDM32)
		      && entry->key_.symndx == 0);

	  ++arg->n_ldm_entries;
	}
    }
  else
    /* This entry is for local symbol.  */
    entry->u.s2.next = NULL;

  return 1;
}

/* Assign offsets within GOT.  USE_NEG_GOT_OFFSETS_P indicates if we
   should use negative offsets.
   Build list of GOT entries for global symbols along the way.
   SYMNDX2H is mapping from global symbol indices to actual
   global symbols.
   Return offset at which next GOT should start.  */

static void
elf_m68k_finalize_got_offsets (struct elf_m68k_got *got,
			       bool use_neg_got_offsets_p,
			       struct elf_m68k_link_hash_entry **symndx2h,
			       bfd_vma *final_offset, bfd_vma *n_ldm_entries)
{
  struct elf_m68k_finalize_got_offsets_arg arg_;
  bfd_vma offset1_[2 * R_LAST];
  bfd_vma offset2_[2 * R_LAST];
  int i;
  bfd_vma start_offset;

  BFD_ASSERT (got->offset != (bfd_vma) -1);

  /* We set entry offsets relative to the .got section (and not the
     start of a particular GOT), so that we can use them in
     finish_dynamic_symbol without needing to know the GOT which they come
     from.  */

  /* Put offset1 in the middle of offset1_, same for offset2.  */
  arg_.offset1 = offset1_ + R_LAST;
  arg_.offset2 = offset2_ + R_LAST;

  start_offset = got->offset;

  if (use_neg_got_offsets_p)
    /* Setup both negative and positive ranges for R_8, R_16 and R_32.  */
    i = -(int) R_32 - 1;
  else
    /* Setup positives ranges for R_8, R_16 and R_32.  */
    i = (int) R_8;

  for (; i <= (int) R_32; ++i)
    {
      int j;
      size_t n;

      /* Set beginning of the range of offsets I.  */
      arg_.offset1[i] = start_offset;

      /* Calculate number of slots that require I offsets.  */
      j = (i >= 0) ? i : -i - 1;
      n = (j >= 1) ? got->n_slots[j - 1] : 0;
      n = got->n_slots[j] - n;

      if (use_neg_got_offsets_p && n != 0)
	{
	  if (i < 0)
	    /* We first fill the positive side of the range, so we might
	       end up with one empty slot at that side when we can't fit
	       whole 2-slot entry.  Account for that at negative side of
	       the interval with one additional entry.  */
	    n = n / 2 + 1;
	  else
	    /* When the number of slots is odd, make positive side of the
	       range one entry bigger.  */
	    n = (n + 1) / 2;
	}

      /* N is the number of slots that require I offsets.
	 Calculate length of the range for I offsets.  */
      n = 4 * n;

      /* Set end of the range.  */
      arg_.offset2[i] = start_offset + n;

      start_offset = arg_.offset2[i];
    }

  if (!use_neg_got_offsets_p)
    /* Make sure that if we try to switch to negative offsets in
       elf_m68k_finalize_got_offsets_1, the assert therein will catch
       the bug.  */
    for (i = R_8; i <= R_32; ++i)
      arg_.offset2[-i - 1] = arg_.offset2[i];

  /* Setup got->offset.  offset1[R_8] is either in the middle or at the
     beginning of GOT depending on use_neg_got_offsets_p.  */
  got->offset = arg_.offset1[R_8];

  arg_.symndx2h = symndx2h;
  arg_.n_ldm_entries = 0;

  /* Assign offsets.  */
  htab_traverse (got->entries, elf_m68k_finalize_got_offsets_1, &arg_);

  /* Check offset ranges we have actually assigned.  */
  for (i = (int) R_8; i <= (int) R_32; ++i)
    BFD_ASSERT (arg_.offset2[i] - arg_.offset1[i] <= 4);

  *final_offset = start_offset;
  *n_ldm_entries = arg_.n_ldm_entries;
}

struct elf_m68k_partition_multi_got_arg
{
  /* The GOT we are adding entries to.  Aka big got.  */
  struct elf_m68k_got *current_got;

  /* Offset to assign the next CURRENT_GOT.  */
  bfd_vma offset;

  /* Context where memory should be allocated.  */
  struct bfd_link_info *info;

  /* Total number of slots in the .got section.
     This is used to calculate size of the .got and .rela.got sections.  */
  bfd_vma n_slots;

  /* Difference in numbers of allocated slots in the .got section
     and necessary relocations in the .rela.got section.
     This is used to calculate size of the .rela.got section.  */
  bfd_vma slots_relas_diff;

  /* Error flag.  */
  bool error_p;

  /* Mapping from global symndx to global symbols.
     This is used to build lists of got entries for global symbols.  */
  struct elf_m68k_link_hash_entry **symndx2h;
};

static void
elf_m68k_partition_multi_got_2 (struct elf_m68k_partition_multi_got_arg *arg)
{
  bfd_vma n_ldm_entries;

  elf_m68k_finalize_got_offsets (arg->current_got,
				 (elf_m68k_hash_table (arg->info)
				  ->use_neg_got_offsets_p),
				 arg->symndx2h,
				 &arg->offset, &n_ldm_entries);

  arg->n_slots += arg->current_got->n_slots[R_32];

  if (!bfd_link_pic (arg->info))
    /* If we are generating a shared object, we need to
       output a R_68K_RELATIVE reloc so that the dynamic
       linker can adjust this GOT entry.  Overwise we
       don't need space in .rela.got for local symbols.  */
    arg->slots_relas_diff += arg->current_got->local_n_slots;

  /* @LDM relocations require a 2-slot GOT entry, but only
     one relocation.  Account for that.  */
  arg->slots_relas_diff += n_ldm_entries;

  BFD_ASSERT (arg->slots_relas_diff <= arg->n_slots);
}


/* Process a single BFD2GOT entry and either merge GOT to CURRENT_GOT
   or start a new CURRENT_GOT.  */

static int
elf_m68k_partition_multi_got_1 (void **_entry, void *_arg)
{
  struct elf_m68k_bfd2got_entry *entry;
  struct elf_m68k_partition_multi_got_arg *arg;
  struct elf_m68k_got *got;
  struct elf_m68k_got diff_;
  struct elf_m68k_got *diff;

  entry = (struct elf_m68k_bfd2got_entry *) *_entry;
  arg = (struct elf_m68k_partition_multi_got_arg *) _arg;

  got = entry->got;
  BFD_ASSERT (got != NULL);
  BFD_ASSERT (got->offset == (bfd_vma) -1);

  diff = NULL;

  if (arg->current_got != NULL)
    /* Construct diff.  */
    {
      diff = &diff_;
      elf_m68k_init_got (diff);

      if (!elf_m68k_can_merge_gots (arg->current_got, got, arg->info, diff))
	{
	  if (diff->offset == 0)
	    /* Offset set to 0 in the diff_ indicates an error.  */
	    {
	      arg->error_p = true;
	      goto final_return;
	    }

	  if (elf_m68k_hash_table (arg->info)->allow_multigot_p)
	    {
	      elf_m68k_clear_got (diff);
	      /* Schedule to finish up current_got and start new one.  */
	      diff = NULL;
	    }
	  /* else
	     Merge GOTs no matter what.  If big GOT overflows,
	     we'll fail in relocate_section due to truncated relocations.

	     ??? May be fail earlier?  E.g., in can_merge_gots.  */
	}
    }
  else
    /* Diff of got against empty current_got is got itself.  */
    {
      /* Create empty current_got to put subsequent GOTs to.  */
      arg->current_got = elf_m68k_create_empty_got (arg->info);
      if (arg->current_got == NULL)
	{
	  arg->error_p = true;
	  goto final_return;
	}

      arg->current_got->offset = arg->offset;

      diff = got;
    }

  if (diff != NULL)
    {
      if (!elf_m68k_merge_gots (arg->current_got, diff, arg->info))
	{
	  arg->error_p = true;
	  goto final_return;
	}

      /* Now we can free GOT.  */
      elf_m68k_clear_got (got);

      entry->got = arg->current_got;
    }
  else
    {
      /* Finish up current_got.  */
      elf_m68k_partition_multi_got_2 (arg);

      /* Schedule to start a new current_got.  */
      arg->current_got = NULL;

      /* Retry.  */
      if (!elf_m68k_partition_multi_got_1 (_entry, _arg))
	{
	  BFD_ASSERT (arg->error_p);
	  goto final_return;
	}
    }

 final_return:
  if (diff != NULL)
    elf_m68k_clear_got (diff);

  return !arg->error_p;
}

/* Helper function to build symndx2h mapping.  */

static bool
elf_m68k_init_symndx2h_1 (struct elf_link_hash_entry *_h,
			  void *_arg)
{
  struct elf_m68k_link_hash_entry *h;

  h = elf_m68k_hash_entry (_h);

  if (h->got_entry_key != 0)
    /* H has at least one entry in the GOT.  */
    {
      struct elf_m68k_partition_multi_got_arg *arg;

      arg = (struct elf_m68k_partition_multi_got_arg *) _arg;

      BFD_ASSERT (arg->symndx2h[h->got_entry_key] == NULL);
      arg->symndx2h[h->got_entry_key] = h;
    }

  return true;
}

/* Merge GOTs of some BFDs, assign offsets to GOT entries and build
   lists of GOT entries for global symbols.
   Calculate sizes of .got and .rela.got sections.  */

static bool
elf_m68k_partition_multi_got (struct bfd_link_info *info)
{
  struct elf_m68k_multi_got *multi_got;
  struct elf_m68k_partition_multi_got_arg arg_;

  multi_got = elf_m68k_multi_got (info);

  arg_.current_got = NULL;
  arg_.offset = 0;
  arg_.info = info;
  arg_.n_slots = 0;
  arg_.slots_relas_diff = 0;
  arg_.error_p = false;

  if (multi_got->bfd2got != NULL)
    {
      /* Initialize symndx2h mapping.  */
      {
	arg_.symndx2h = bfd_zmalloc (multi_got->global_symndx
				     * sizeof (*arg_.symndx2h));
	if (arg_.symndx2h == NULL)
	  return false;

	elf_link_hash_traverse (elf_hash_table (info),
				elf_m68k_init_symndx2h_1, &arg_);
      }

      /* Partition.  */
      htab_traverse (multi_got->bfd2got, elf_m68k_partition_multi_got_1,
		     &arg_);
      if (arg_.error_p)
	{
	  free (arg_.symndx2h);
	  arg_.symndx2h = NULL;

	  return false;
	}

      /* Finish up last current_got.  */
      elf_m68k_partition_multi_got_2 (&arg_);

      free (arg_.symndx2h);
    }

  if (elf_hash_table (info)->dynobj != NULL)
    /* Set sizes of .got and .rela.got sections.  */
    {
      asection *s;

      s = elf_hash_table (info)->sgot;
      if (s != NULL)
	s->size = arg_.offset;
      else
	BFD_ASSERT (arg_.offset == 0);

      BFD_ASSERT (arg_.slots_relas_diff <= arg_.n_slots);
      arg_.n_slots -= arg_.slots_relas_diff;

      s = elf_hash_table (info)->srelgot;
      if (s != NULL)
	s->size = arg_.n_slots * sizeof (Elf32_External_Rela);
      else
	BFD_ASSERT (arg_.n_slots == 0);
    }
  else
    BFD_ASSERT (multi_got->bfd2got == NULL);

  return true;
}

/* Copy any information related to dynamic linking from a pre-existing
   symbol to a newly created symbol.  Also called to copy flags and
   other back-end info to a weakdef, in which case the symbol is not
   newly created and plt/got refcounts and dynamic indices should not
   be copied.  */

static void
elf_m68k_copy_indirect_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *_dir,
			       struct elf_link_hash_entry *_ind)
{
  struct elf_m68k_link_hash_entry *dir;
  struct elf_m68k_link_hash_entry *ind;

  _bfd_elf_link_hash_copy_indirect (info, _dir, _ind);

  if (_ind->root.type != bfd_link_hash_indirect)
    return;

  dir = elf_m68k_hash_entry (_dir);
  ind = elf_m68k_hash_entry (_ind);

  /* Any absolute non-dynamic relocations against an indirect or weak
     definition will be against the target symbol.  */
  _dir->non_got_ref |= _ind->non_got_ref;

  /* We might have a direct symbol already having entries in the GOTs.
     Update its key only in case indirect symbol has GOT entries and
     assert that both indirect and direct symbols don't have GOT entries
     at the same time.  */
  if (ind->got_entry_key != 0)
    {
      BFD_ASSERT (dir->got_entry_key == 0);
      /* Assert that GOTs aren't partioned yet.  */
      BFD_ASSERT (ind->glist == NULL);

      dir->got_entry_key = ind->got_entry_key;
      ind->got_entry_key = 0;
    }
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bool
elf_m68k_check_relocs (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  bfd *dynobj;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  struct elf_m68k_got *got;

  if (bfd_link_relocatable (info))
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  sreloc = NULL;

  got = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;

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

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_68K_GOT8:
	case R_68K_GOT16:
	case R_68K_GOT32:
	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    break;
	  /* Fall through.  */

	  /* Relative GOT relocations.  */
	case R_68K_GOT8O:
	case R_68K_GOT16O:
	case R_68K_GOT32O:
	  /* Fall through.  */

	  /* TLS relocations.  */
	case R_68K_TLS_GD8:
	case R_68K_TLS_GD16:
	case R_68K_TLS_GD32:
	case R_68K_TLS_LDM8:
	case R_68K_TLS_LDM16:
	case R_68K_TLS_LDM32:
	case R_68K_TLS_IE8:
	case R_68K_TLS_IE16:
	case R_68K_TLS_IE32:

	case R_68K_TLS_TPREL32:
	case R_68K_TLS_DTPREL32:

	  if (ELF32_R_TYPE (rel->r_info) == R_68K_TLS_TPREL32
	      && bfd_link_pic (info))
	    /* Do the special chorus for libraries with static TLS.  */
	    info->flags |= DF_STATIC_TLS;

	  /* This symbol requires a global offset table entry.  */

	  if (dynobj == NULL)
	    {
	      /* Create the .got section.  */
	      elf_hash_table (info)->dynobj = dynobj = abfd;
	      if (!_bfd_elf_create_got_section (dynobj, info))
		return false;
	    }

	  if (got == NULL)
	    {
	      struct elf_m68k_bfd2got_entry *bfd2got_entry;

	      bfd2got_entry
		= elf_m68k_get_bfd2got_entry (elf_m68k_multi_got (info),
					      abfd, FIND_OR_CREATE, info);
	      if (bfd2got_entry == NULL)
		return false;

	      got = bfd2got_entry->got;
	      BFD_ASSERT (got != NULL);
	    }

	  {
	    struct elf_m68k_got_entry *got_entry;

	    /* Add entry to got.  */
	    got_entry = elf_m68k_add_entry_to_got (got, h, abfd,
						   ELF32_R_TYPE (rel->r_info),
						   r_symndx, info);
	    if (got_entry == NULL)
	      return false;

	    if (got_entry->u.s1.refcount == 1)
	      {
		/* Make sure this symbol is output as a dynamic symbol.  */
		if (h != NULL
		    && h->dynindx == -1
		    && !h->forced_local)
		  {
		    if (!bfd_elf_link_record_dynamic_symbol (info, h))
		      return false;
		  }
	      }
	  }

	  break;

	case R_68K_PLT8:
	case R_68K_PLT16:
	case R_68K_PLT32:
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
	  h->plt.refcount++;
	  break;

	case R_68K_PLT8O:
	case R_68K_PLT16O:
	case R_68K_PLT32O:
	  /* This symbol requires a procedure linkage table entry.  */

	  if (h == NULL)
	    {
	      /* It does not make sense to have this relocation for a
		 local symbol.  FIXME: does it?  How to handle it if
		 it does make sense?  */
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }

	  /* Make sure this symbol is output as a dynamic symbol.  */
	  if (h->dynindx == -1
	      && !h->forced_local)
	    {
	      if (!bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }

	  h->needs_plt = 1;
	  h->plt.refcount++;
	  break;

	case R_68K_PC8:
	case R_68K_PC16:
	case R_68K_PC32:
	  /* If we are creating a shared library and this is not a local
	     symbol, we need to copy the reloc into the shared library.
	     However when linking with -Bsymbolic and this is a global
	     symbol which is defined in an object we are including in the
	     link (i.e., DEF_REGULAR is set), then we can resolve the
	     reloc directly.  At this point we have not seen all the input
	     files, so it is possible that DEF_REGULAR is not set now but
	     will be set later (it is never cleared).  We account for that
	     possibility below by storing information in the
	     pcrel_relocs_copied field of the hash table entry.  */
	  if (!(bfd_link_pic (info)
		&& (sec->flags & SEC_ALLOC) != 0
		&& h != NULL
		&& (!SYMBOLIC_BIND (info, h)
		    || h->root.type == bfd_link_hash_defweak
		    || !h->def_regular)))
	    {
	      if (h != NULL)
		{
		  /* Make sure a plt entry is created for this symbol if
		     it turns out to be a function defined by a dynamic
		     object.  */
		  h->plt.refcount++;
		}
	      break;
	    }
	  /* Fall through.  */
	case R_68K_8:
	case R_68K_16:
	case R_68K_32:
	  /* We don't need to handle relocs into sections not going into
	     the "real" output.  */
	  if ((sec->flags & SEC_ALLOC) == 0)
	      break;

	  if (h != NULL)
	    {
	      /* Make sure a plt entry is created for this symbol if it
		 turns out to be a function defined by a dynamic object.  */
	      h->plt.refcount++;

	      if (bfd_link_executable (info))
		/* This symbol needs a non-GOT reference.  */
		h->non_got_ref = 1;
	    }

	  /* If we are creating a shared library, we need to copy the
	     reloc into the shared library.  */
	  if (bfd_link_pic (info)
	      && (h == NULL
		  || !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h)))
	    {
	      /* When creating a shared object, we must copy these
		 reloc types into the output file.  We create a reloc
		 section in dynobj and make room for this reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, dynobj, 2, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    return false;
		}

	      if (sec->flags & SEC_READONLY
		  /* Don't set DF_TEXTREL yet for PC relative
		     relocations, they might be discarded later.  */
		  && !(ELF32_R_TYPE (rel->r_info) == R_68K_PC8
		       || ELF32_R_TYPE (rel->r_info) == R_68K_PC16
		       || ELF32_R_TYPE (rel->r_info) == R_68K_PC32))
		    info->flags |= DF_TEXTREL;

	      sreloc->size += sizeof (Elf32_External_Rela);

	      /* We count the number of PC relative relocations we have
		 entered for this symbol, so that we can discard them
		 again if, in the -Bsymbolic case, the symbol is later
		 defined by a regular object, or, in the normal shared
		 case, the symbol is forced to be local.  Note that this
		 function is only called if we are using an m68kelf linker
		 hash table, which means that h is really a pointer to an
		 elf_m68k_link_hash_entry.  */
	      if (ELF32_R_TYPE (rel->r_info) == R_68K_PC8
		  || ELF32_R_TYPE (rel->r_info) == R_68K_PC16
		  || ELF32_R_TYPE (rel->r_info) == R_68K_PC32)
		{
		  struct elf_m68k_pcrel_relocs_copied *p;
		  struct elf_m68k_pcrel_relocs_copied **head;

		  if (h != NULL)
		    {
		      struct elf_m68k_link_hash_entry *eh
			= elf_m68k_hash_entry (h);
		      head = &eh->pcrel_relocs_copied;
		    }
		  else
		    {
		      asection *s;
		      void *vpp;
		      Elf_Internal_Sym *isym;

		      isym = bfd_sym_from_r_symndx (&elf_m68k_hash_table (info)->root.sym_cache,
						    abfd, r_symndx);
		      if (isym == NULL)
			return false;

		      s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		      if (s == NULL)
			s = sec;

		      vpp = &elf_section_data (s)->local_dynrel;
		      head = (struct elf_m68k_pcrel_relocs_copied **) vpp;
		    }

		  for (p = *head; p != NULL; p = p->next)
		    if (p->section == sreloc)
		      break;

		  if (p == NULL)
		    {
		      p = ((struct elf_m68k_pcrel_relocs_copied *)
			   bfd_alloc (dynobj, (bfd_size_type) sizeof *p));
		      if (p == NULL)
			return false;
		      p->next = *head;
		      *head = p;
		      p->section = sreloc;
		      p->count = 0;
		    }

		  ++p->count;
		}
	    }

	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_68K_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_68K_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
elf_m68k_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_68K_GNU_VTINHERIT:
      case R_68K_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Return the type of PLT associated with OUTPUT_BFD.  */

static const struct elf_m68k_plt_info *
elf_m68k_get_plt_info (bfd *output_bfd)
{
  unsigned int features;

  features = bfd_m68k_mach_to_features (bfd_get_mach (output_bfd));
  if (features & cpu32)
    return &elf_cpu32_plt_info;
  if (features & mcfisa_b)
    return &elf_isab_plt_info;
  if (features & mcfisa_c)
    return &elf_isac_plt_info;
  return &elf_m68k_plt_info;
}

/* This function is called after all the input files have been read,
   and the input sections have been assigned to output sections.
   It's a convenient place to determine the PLT style.  */

static bool
elf_m68k_always_size_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  /* Bind input BFDs to GOTs and calculate sizes of .got and .rela.got
     sections.  */
  if (!elf_m68k_partition_multi_got (info))
    return false;

  elf_m68k_hash_table (info)->plt_info = elf_m68k_get_plt_info (output_bfd);
  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf_m68k_adjust_dynamic_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h)
{
  struct elf_m68k_link_hash_table *htab;
  bfd *dynobj;
  asection *s;

  htab = elf_m68k_hash_table (info);
  dynobj = htab->root.dynobj;

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
      if ((h->plt.refcount <= 0
	   || SYMBOL_CALLS_LOCAL (info, h)
	   || ((ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		|| UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	       && h->root.type == bfd_link_hash_undefweak))
	  /* We must always create the plt entry if it was referenced
	     by a PLTxxO relocation.  In this case we already recorded
	     it as a dynamic symbol.  */
	  && h->dynindx == -1)
	{
	  /* This case can occur if we saw a PLTxx reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PCxx reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	  return true;
	}

      /* Make sure this symbol is output as a dynamic symbol.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->root.splt;
      BFD_ASSERT (s != NULL);

      /* If this is the first .plt entry, make room for the special
	 first entry.  */
      if (s->size == 0)
	s->size = htab->plt_info->size;

      /* If this symbol is not defined in a regular file, and we are
	 not generating a shared library, then set the symbol to this
	 location in the .plt.  This is required to make function
	 pointers compare as equal between the normal executable and
	 the shared library.  */
      if (!bfd_link_pic (info)
	  && !h->def_regular)
	{
	  h->root.u.def.section = s;
	  h->root.u.def.value = s->size;
	}

      h->plt.offset = s->size;

      /* Make room for this entry.  */
      s->size += htab->plt_info->size;

      /* We also need to make an entry in the .got.plt section, which
	 will be placed in the .got section by the linker script.  */
      s = htab->root.sgotplt;
      BFD_ASSERT (s != NULL);
      s->size += 4;

      /* We also need to make an entry in the .rela.plt section.  */
      s = htab->root.srelplt;
      BFD_ASSERT (s != NULL);
      s->size += sizeof (Elf32_External_Rela);

      return true;
    }

  /* Reinitialize the plt offset now that it is not used as a reference
     count any more.  */
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

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  s = bfd_get_linker_section (dynobj, ".dynbss");
  BFD_ASSERT (s != NULL);

  /* We must generate a R_68K_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      srel = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Set the sizes of the dynamic sections.  */

static bool
elf_m68k_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bool relocs;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

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
  else
    {
      /* We may have created entries in the .rela.got section.
	 However, if we are not creating the dynamic sections, we will
	 not actually use these entries.  Reset the size of .rela.got,
	 which will cause it to get stripped from the output file
	 below.  */
      s = elf_hash_table (info)->srelgot;
      if (s != NULL)
	s->size = 0;
    }

  /* If this is a -Bsymbolic shared link, then we need to discard all
     PC relative relocs against symbols defined in a regular object.
     For the normal shared case we discard the PC relative relocs
     against symbols that have become local due to visibility changes.
     We allocated space for them in the check_relocs routine, but we
     will not fill them in in the relocate_section routine.  */
  if (bfd_link_pic (info))
    elf_link_hash_traverse (elf_hash_table (info),
			    elf_m68k_discard_copies,
			    info);

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if (strcmp (name, ".plt") == 0)
	{
	  /* Remember whether there is a PLT.  */
	  ;
	}
      else if (startswith (name, ".rela"))
	{
	  if (s->size != 0)
	    {
	      relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (! startswith (name, ".got")
	       && strcmp (name, ".dynbss") != 0)
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
      /* FIXME: This should be a call to bfd_alloc not bfd_zalloc.
	 Unused entries should be reclaimed before the section's contents
	 are written out, but at the moment this does not happen.  Thus in
	 order to prevent writing out garbage, we initialise the section's
	 contents to zero.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* This function is called via elf_link_hash_traverse if we are
   creating a shared object.  In the -Bsymbolic case it discards the
   space allocated to copy PC relative relocs against symbols which
   are defined in regular objects.  For the normal shared case, it
   discards space for pc-relative relocs that have become local due to
   symbol visibility changes.  We allocated space for them in the
   check_relocs routine, but we won't fill them in in the
   relocate_section routine.

   We also check whether any of the remaining relocations apply
   against a readonly section, and set the DF_TEXTREL flag in this
   case.  */

static bool
elf_m68k_discard_copies (struct elf_link_hash_entry *h,
			 void * inf)
{
  struct bfd_link_info *info = (struct bfd_link_info *) inf;
  struct elf_m68k_pcrel_relocs_copied *s;

  if (!SYMBOL_CALLS_LOCAL (info, h))
    {
      if ((info->flags & DF_TEXTREL) == 0)
	{
	  /* Look for relocations against read-only sections.  */
	  for (s = elf_m68k_hash_entry (h)->pcrel_relocs_copied;
	       s != NULL;
	       s = s->next)
	    if ((s->section->flags & SEC_READONLY) != 0)
	      {
		info->flags |= DF_TEXTREL;
		break;
	      }
	}

      /* Make sure undefined weak symbols are output as a dynamic symbol
	 in PIEs.  */
      if (h->non_got_ref
	  && h->root.type == bfd_link_hash_undefweak
	  && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	  && h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      return true;
    }

  for (s = elf_m68k_hash_entry (h)->pcrel_relocs_copied;
       s != NULL;
       s = s->next)
    s->section->size -= s->count * sizeof (Elf32_External_Rela);

  return true;
}


/* Install relocation RELA.  */

static void
elf_m68k_install_rela (bfd *output_bfd,
		       asection *srela,
		       Elf_Internal_Rela *rela)
{
  bfd_byte *loc;

  loc = srela->contents;
  loc += srela->reloc_count++ * sizeof (Elf32_External_Rela);
  bfd_elf32_swap_reloca_out (output_bfd, rela, loc);
}

/* Find the base offsets for thread-local storage in this object,
   for GD/LD and IE/LE respectively.  */

#define DTP_OFFSET 0x8000
#define TP_OFFSET  0x7000

static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma + DTP_OFFSET;
}

static bfd_vma
tpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma + TP_OFFSET;
}

/* Output necessary relocation to handle a symbol during static link.
   This function is called from elf_m68k_relocate_section.  */

static void
elf_m68k_init_got_entry_static (struct bfd_link_info *info,
				bfd *output_bfd,
				enum elf_m68k_reloc_type r_type,
				asection *sgot,
				bfd_vma got_entry_offset,
				bfd_vma relocation)
{
  switch (elf_m68k_reloc_got_type (r_type))
    {
    case R_68K_GOT32O:
      bfd_put_32 (output_bfd, relocation, sgot->contents + got_entry_offset);
      break;

    case R_68K_TLS_GD32:
      /* We know the offset within the module,
	 put it into the second GOT slot.  */
      bfd_put_32 (output_bfd, relocation - dtpoff_base (info),
		  sgot->contents + got_entry_offset + 4);
      /* FALLTHRU */

    case R_68K_TLS_LDM32:
      /* Mark it as belonging to module 1, the executable.  */
      bfd_put_32 (output_bfd, 1, sgot->contents + got_entry_offset);
      break;

    case R_68K_TLS_IE32:
      bfd_put_32 (output_bfd, relocation - tpoff_base (info),
		  sgot->contents + got_entry_offset);
      break;

    default:
      BFD_ASSERT (false);
    }
}

/* Output necessary relocation to handle a local symbol
   during dynamic link.
   This function is called either from elf_m68k_relocate_section
   or from elf_m68k_finish_dynamic_symbol.  */

static void
elf_m68k_init_got_entry_local_shared (struct bfd_link_info *info,
				      bfd *output_bfd,
				      enum elf_m68k_reloc_type r_type,
				      asection *sgot,
				      bfd_vma got_entry_offset,
				      bfd_vma relocation,
				      asection *srela)
{
  Elf_Internal_Rela outrel;

  switch (elf_m68k_reloc_got_type (r_type))
    {
    case R_68K_GOT32O:
      /* Emit RELATIVE relocation to initialize GOT slot
	 at run-time.  */
      outrel.r_info = ELF32_R_INFO (0, R_68K_RELATIVE);
      outrel.r_addend = relocation;
      break;

    case R_68K_TLS_GD32:
      /* We know the offset within the module,
	 put it into the second GOT slot.  */
      bfd_put_32 (output_bfd, relocation - dtpoff_base (info),
		  sgot->contents + got_entry_offset + 4);
      /* FALLTHRU */

    case R_68K_TLS_LDM32:
      /* We don't know the module number,
	 create a relocation for it.  */
      outrel.r_info = ELF32_R_INFO (0, R_68K_TLS_DTPMOD32);
      outrel.r_addend = 0;
      break;

    case R_68K_TLS_IE32:
      /* Emit TPREL relocation to initialize GOT slot
	 at run-time.  */
      outrel.r_info = ELF32_R_INFO (0, R_68K_TLS_TPREL32);
      outrel.r_addend = relocation - elf_hash_table (info)->tls_sec->vma;
      break;

    default:
      BFD_ASSERT (false);
    }

  /* Offset of the GOT entry.  */
  outrel.r_offset = (sgot->output_section->vma
		     + sgot->output_offset
		     + got_entry_offset);

  /* Install one of the above relocations.  */
  elf_m68k_install_rela (output_bfd, srela, &outrel);

  bfd_put_32 (output_bfd, outrel.r_addend, sgot->contents + got_entry_offset);
}

/* Relocate an M68K ELF section.  */

static int
elf_m68k_relocate_section (bfd *output_bfd,
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
  asection *sgot;
  asection *splt;
  asection *sreloc;
  asection *srela;
  struct elf_m68k_got *got;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  sgot = NULL;
  splt = NULL;
  sreloc = NULL;
  srela = NULL;

  got = NULL;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma relocation;
      bool unresolved_reloc;
      bfd_reloc_status_type r;
      bool resolved_to_zero;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type < 0 || r_type >= (int) R_68K_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      howto = howto_table + r_type;

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
	continue;

      resolved_to_zero = (h != NULL
			  && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

      switch (r_type)
	{
	case R_68K_GOT8:
	case R_68K_GOT16:
	case R_68K_GOT32:
	  /* Relocation is to the address of the entry for this symbol
	     in the global offset table.  */
	  if (h != NULL
	      && strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
	    {
	      if (elf_m68k_hash_table (info)->local_gp_p)
		{
		  bfd_vma sgot_output_offset;
		  bfd_vma got_offset;

		  sgot = elf_hash_table (info)->sgot;

		  if (sgot != NULL)
		    sgot_output_offset = sgot->output_offset;
		  else
		    /* In this case we have a reference to
		       _GLOBAL_OFFSET_TABLE_, but the GOT itself is
		       empty.
		       ??? Issue a warning?  */
		    sgot_output_offset = 0;

		  if (got == NULL)
		    {
		      struct elf_m68k_bfd2got_entry *bfd2got_entry;

		      bfd2got_entry
			= elf_m68k_get_bfd2got_entry (elf_m68k_multi_got (info),
						      input_bfd, SEARCH, NULL);

		      if (bfd2got_entry != NULL)
			{
			  got = bfd2got_entry->got;
			  BFD_ASSERT (got != NULL);

			  got_offset = got->offset;
			}
		      else
			/* In this case we have a reference to
			   _GLOBAL_OFFSET_TABLE_, but no other references
			   accessing any GOT entries.
			   ??? Issue a warning?  */
			got_offset = 0;
		    }
		  else
		    got_offset = got->offset;

		  /* Adjust GOT pointer to point to the GOT
		     assigned to input_bfd.  */
		  rel->r_addend += sgot_output_offset + got_offset;
		}
	      else
		BFD_ASSERT (got == NULL || got->offset == 0);

	      break;
	    }
	  /* Fall through.  */
	case R_68K_GOT8O:
	case R_68K_GOT16O:
	case R_68K_GOT32O:

	case R_68K_TLS_LDM32:
	case R_68K_TLS_LDM16:
	case R_68K_TLS_LDM8:

	case R_68K_TLS_GD8:
	case R_68K_TLS_GD16:
	case R_68K_TLS_GD32:

	case R_68K_TLS_IE8:
	case R_68K_TLS_IE16:
	case R_68K_TLS_IE32:

	  /* Relocation is the offset of the entry for this symbol in
	     the global offset table.  */

	  {
	    struct elf_m68k_got_entry_key key_;
	    bfd_vma *off_ptr;
	    bfd_vma off;

	    sgot = elf_hash_table (info)->sgot;
	    BFD_ASSERT (sgot != NULL);

	    if (got == NULL)
	      got = elf_m68k_get_bfd2got_entry (elf_m68k_multi_got (info),
						input_bfd, MUST_FIND,
						NULL)->got;

	    /* Get GOT offset for this symbol.  */
	    elf_m68k_init_got_entry_key (&key_, h, input_bfd, r_symndx,
					 r_type);
	    off_ptr = &elf_m68k_get_got_entry (got, &key_, MUST_FIND,
					       NULL)->u.s2.offset;
	    off = *off_ptr;

	    /* The offset must always be a multiple of 4.  We use
	       the least significant bit to record whether we have
	       already generated the necessary reloc.  */
	    if ((off & 1) != 0)
	      off &= ~1;
	    else
	      {
		if (h != NULL
		    /* @TLSLDM relocations are bounded to the module, in
		       which the symbol is defined -- not to the symbol
		       itself.  */
		    && elf_m68k_reloc_got_type (r_type) != R_68K_TLS_LDM32)
		  {
		    bool dyn;

		    dyn = elf_hash_table (info)->dynamic_sections_created;
		    if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							  bfd_link_pic (info),
							  h)
			|| (bfd_link_pic (info)
			    && SYMBOL_REFERENCES_LOCAL (info, h))
			|| ((ELF_ST_VISIBILITY (h->other)
			     || resolved_to_zero)
			    && h->root.type == bfd_link_hash_undefweak))
		      {
			/* This is actually a static link, or it is a
			   -Bsymbolic link and the symbol is defined
			   locally, or the symbol was forced to be local
			   because of a version file.  We must initialize
			   this entry in the global offset table.  Since
			   the offset must always be a multiple of 4, we
			   use the least significant bit to record whether
			   we have initialized it already.

			   When doing a dynamic link, we create a .rela.got
			   relocation entry to initialize the value.  This
			   is done in the finish_dynamic_symbol routine.  */

			elf_m68k_init_got_entry_static (info,
							output_bfd,
							r_type,
							sgot,
							off,
							relocation);

			*off_ptr |= 1;
		      }
		    else
		      unresolved_reloc = false;
		  }
		else if (bfd_link_pic (info)) /* && h == NULL */
		  /* Process local symbol during dynamic link.  */
		  {
		    srela = elf_hash_table (info)->srelgot;
		    BFD_ASSERT (srela != NULL);

		    elf_m68k_init_got_entry_local_shared (info,
							  output_bfd,
							  r_type,
							  sgot,
							  off,
							  relocation,
							  srela);

		    *off_ptr |= 1;
		  }
		else /* h == NULL && !bfd_link_pic (info) */
		  {
		    elf_m68k_init_got_entry_static (info,
						    output_bfd,
						    r_type,
						    sgot,
						    off,
						    relocation);

		    *off_ptr |= 1;
		  }
	      }

	    /* We don't use elf_m68k_reloc_got_type in the condition below
	       because this is the only place where difference between
	       R_68K_GOTx and R_68K_GOTxO relocations matters.  */
	    if (r_type == R_68K_GOT32O
		|| r_type == R_68K_GOT16O
		|| r_type == R_68K_GOT8O
		|| elf_m68k_reloc_got_type (r_type) == R_68K_TLS_GD32
		|| elf_m68k_reloc_got_type (r_type) == R_68K_TLS_LDM32
		|| elf_m68k_reloc_got_type (r_type) == R_68K_TLS_IE32)
	      {
		/* GOT pointer is adjusted to point to the start/middle
		   of local GOT.  Adjust the offset accordingly.  */
		BFD_ASSERT (elf_m68k_hash_table (info)->use_neg_got_offsets_p
			    || off >= got->offset);

		if (elf_m68k_hash_table (info)->local_gp_p)
		  relocation = off - got->offset;
		else
		  {
		    BFD_ASSERT (got->offset == 0);
		    relocation = sgot->output_offset + off;
		  }

		/* This relocation does not use the addend.  */
		rel->r_addend = 0;
	      }
	    else
	      relocation = (sgot->output_section->vma + sgot->output_offset
			    + off);
	  }
	  break;

	case R_68K_TLS_LDO32:
	case R_68K_TLS_LDO16:
	case R_68K_TLS_LDO8:
	  relocation -= dtpoff_base (info);
	  break;

	case R_68K_TLS_LE32:
	case R_68K_TLS_LE16:
	case R_68K_TLS_LE8:
	  if (bfd_link_dll (info))
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
	    relocation -= tpoff_base (info);

	  break;

	case R_68K_PLT8:
	case R_68K_PLT16:
	case R_68K_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLTxx reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL)
	    break;

	  if (h->plt.offset == (bfd_vma) -1
	      || !elf_hash_table (info)->dynamic_sections_created)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  splt = elf_hash_table (info)->splt;
	  BFD_ASSERT (splt != NULL);

	  relocation = (splt->output_section->vma
			+ splt->output_offset
			+ h->plt.offset);
	  unresolved_reloc = false;
	  break;

	case R_68K_PLT8O:
	case R_68K_PLT16O:
	case R_68K_PLT32O:
	  /* Relocation is the offset of the entry for this symbol in
	     the procedure linkage table.  */
	  BFD_ASSERT (h != NULL && h->plt.offset != (bfd_vma) -1);

	  splt = elf_hash_table (info)->splt;
	  BFD_ASSERT (splt != NULL);

	  relocation = h->plt.offset;
	  unresolved_reloc = false;

	  /* This relocation does not use the addend.  */
	  rel->r_addend = 0;

	  break;

	case R_68K_8:
	case R_68K_16:
	case R_68K_32:
	case R_68K_PC8:
	case R_68K_PC16:
	case R_68K_PC32:
	  if (bfd_link_pic (info)
	      && r_symndx != STN_UNDEF
	      && (input_section->flags & SEC_ALLOC) != 0
	      && (h == NULL
		  || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		      && !resolved_to_zero)
		  || h->root.type != bfd_link_hash_undefweak)
	      && ((r_type != R_68K_PC8
		   && r_type != R_68K_PC16
		   && r_type != R_68K_PC32)
		  || !SYMBOL_CALLS_LOCAL (info, h)))
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      bool skip, relocate;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

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
		       && (r_type == R_68K_PC8
			   || r_type == R_68K_PC16
			   || r_type == R_68K_PC32
			   || !bfd_link_pic (info)
			   || !SYMBOLIC_BIND (info, h)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  outrel.r_addend = relocation + rel->r_addend;

		  if (r_type == R_68K_32)
		    {
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_68K_RELATIVE);
		    }
		  else
		    {
		      long indx;

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
			      struct elf_link_hash_table *htab;
			      htab = elf_hash_table (info);
			      osec = htab->text_index_section;
			      indx = elf_section_data (osec)->dynindx;
			    }
			  BFD_ASSERT (indx != 0);
			}

		      outrel.r_info = ELF32_R_INFO (indx, r_type);
		    }
		}

	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      /* This reloc will be computed at runtime, so there's no
		 need to do anything now, except for R_68K_32
		 relocations that have been turned into
		 R_68K_RELATIVE.  */
	      if (!relocate)
		continue;
	    }

	  break;

	case R_68K_GNU_VTINHERIT:
	case R_68K_GNU_VTENTRY:
	  /* These are no-ops in the end.  */
	  continue;

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

      if (r_symndx != STN_UNDEF
	  && r_type != R_68K_NONE
	  && (h == NULL
	      || h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak))
	{
	  char sym_type;

	  sym_type = (sym != NULL) ? ELF32_ST_TYPE (sym->st_info) : h->type;

	  if (elf_m68k_reloc_tls_p (r_type) != (sym_type == STT_TLS))
	    {
	      const char *name;

	      if (h != NULL)
		name = h->root.root.string;
	      else
		{
		  name = (bfd_elf_string_from_elf_section
			  (input_bfd, symtab_hdr->sh_link, sym->st_name));
		  if (name == NULL || *name == '\0')
		    name = bfd_section_name (sec);
		}

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
	}

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *name;

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

	  if (r == bfd_reloc_overflow)
	    (*info->callbacks->reloc_overflow)
	      (info, (h ? &h->root : NULL), name, howto->name,
	       (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	  else
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA+%#" PRIx64 "): reloc against `%s': error %d"),
		 input_bfd, input_section,
		 (uint64_t) rel->r_offset, name, (int) r);
	      return false;
	    }
	}
    }

  return true;
}

/* Install an M_68K_PC32 relocation against VALUE at offset OFFSET
   into section SEC.  */

static void
elf_m68k_install_pc32 (asection *sec, bfd_vma offset, bfd_vma value)
{
  /* Make VALUE PC-relative.  */
  value -= sec->output_section->vma + offset;

  /* Apply any in-place addend.  */
  value += bfd_get_32 (sec->owner, sec->contents + offset);

  bfd_put_32 (sec->owner, value, sec->contents + offset);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf_m68k_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  bfd *dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  if (h->plt.offset != (bfd_vma) -1)
    {
      const struct elf_m68k_plt_info *plt_info;
      asection *splt;
      asection *sgot;
      asection *srela;
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */

      BFD_ASSERT (h->dynindx != -1);

      plt_info = elf_m68k_hash_table (info)->plt_info;
      splt = elf_hash_table (info)->splt;
      sgot = elf_hash_table (info)->sgotplt;
      srela = elf_hash_table (info)->srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = (h->plt.offset / plt_info->size) - 1;

      /* Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;

      memcpy (splt->contents + h->plt.offset,
	      plt_info->symbol_entry,
	      plt_info->size);

      elf_m68k_install_pc32 (splt, h->plt.offset + plt_info->symbol_relocs.got,
			     (sgot->output_section->vma
			      + sgot->output_offset
			      + got_offset));

      bfd_put_32 (output_bfd, plt_index * sizeof (Elf32_External_Rela),
		  splt->contents
		  + h->plt.offset
		  + plt_info->symbol_resolve_entry + 2);

      elf_m68k_install_pc32 (splt, h->plt.offset + plt_info->symbol_relocs.plt,
			     splt->output_section->vma);

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset
		   + h->plt.offset
		   + plt_info->symbol_resolve_entry),
		  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_JMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents + plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }

  if (elf_m68k_hash_entry (h)->glist != NULL)
    {
      asection *sgot;
      asection *srela;
      struct elf_m68k_got_entry *got_entry;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      sgot = elf_hash_table (info)->sgot;
      srela = elf_hash_table (info)->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      got_entry = elf_m68k_hash_entry (h)->glist;

      while (got_entry != NULL)
	{
	  enum elf_m68k_reloc_type r_type;
	  bfd_vma got_entry_offset;

	  r_type = got_entry->key_.type;
	  got_entry_offset = got_entry->u.s2.offset &~ (bfd_vma) 1;

	  /* If this is a -Bsymbolic link, and the symbol is defined
	     locally, we just want to emit a RELATIVE reloc.  Likewise if
	     the symbol was forced to be local because of a version file.
	     The entry in the global offset table already have been
	     initialized in the relocate_section function.  */
	  if (bfd_link_pic (info)
	      && SYMBOL_REFERENCES_LOCAL (info, h))
	    {
	      bfd_vma relocation;

	      relocation = bfd_get_signed_32 (output_bfd,
					      (sgot->contents
					       + got_entry_offset));

	      /* Undo TP bias.  */
	      switch (elf_m68k_reloc_got_type (r_type))
		{
		case R_68K_GOT32O:
		case R_68K_TLS_LDM32:
		  break;

		case R_68K_TLS_GD32:
		  /* The value for this relocation is actually put in
		     the second GOT slot.  */
		  relocation = bfd_get_signed_32 (output_bfd,
						  (sgot->contents
						   + got_entry_offset + 4));
		  relocation += dtpoff_base (info);
		  break;

		case R_68K_TLS_IE32:
		  relocation += tpoff_base (info);
		  break;

		default:
		  BFD_ASSERT (false);
		}

	      elf_m68k_init_got_entry_local_shared (info,
						    output_bfd,
						    r_type,
						    sgot,
						    got_entry_offset,
						    relocation,
						    srela);
	    }
	  else
	    {
	      Elf_Internal_Rela rela;

	      /* Put zeros to GOT slots that will be initialized
		 at run-time.  */
	      {
		bfd_vma n_slots;

		n_slots = elf_m68k_reloc_got_n_slots (got_entry->key_.type);
		while (n_slots--)
		  bfd_put_32 (output_bfd, (bfd_vma) 0,
			      (sgot->contents + got_entry_offset
			       + 4 * n_slots));
	      }

	      rela.r_addend = 0;
	      rela.r_offset = (sgot->output_section->vma
			       + sgot->output_offset
			       + got_entry_offset);

	      switch (elf_m68k_reloc_got_type (r_type))
		{
		case R_68K_GOT32O:
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_GLOB_DAT);
		  elf_m68k_install_rela (output_bfd, srela, &rela);
		  break;

		case R_68K_TLS_GD32:
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_TLS_DTPMOD32);
		  elf_m68k_install_rela (output_bfd, srela, &rela);

		  rela.r_offset += 4;
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_TLS_DTPREL32);
		  elf_m68k_install_rela (output_bfd, srela, &rela);
		  break;

		case R_68K_TLS_IE32:
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_TLS_TPREL32);
		  elf_m68k_install_rela (output_bfd, srela, &rela);
		  break;

		default:
		  BFD_ASSERT (false);
		  break;
		}
	    }

	  got_entry = got_entry->u.s2.next;
	}
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_68K_COPY);
      rela.r_addend = 0;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  return true;
}

/* Finish up the dynamic sections.  */

static bool
elf_m68k_finish_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sgot;
  asection *sdyn;

  dynobj = elf_hash_table (info)->dynobj;

  sgot = elf_hash_table (info)->sgotplt;
  BFD_ASSERT (sgot != NULL);
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = elf_hash_table (info)->splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);

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

	    case DT_PLTGOT:
	      s = elf_hash_table (info)->sgotplt;
	      goto get_vma;
	    case DT_JMPREL:
	      s = elf_hash_table (info)->srelplt;
	    get_vma:
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = elf_hash_table (info)->srelplt;
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->size > 0)
	{
	  const struct elf_m68k_plt_info *plt_info;

	  plt_info = elf_m68k_hash_table (info)->plt_info;
	  memcpy (splt->contents, plt_info->plt0_entry, plt_info->size);

	  elf_m68k_install_pc32 (splt, plt_info->plt0_relocs.got4,
				 (sgot->output_section->vma
				  + sgot->output_offset
				  + 4));

	  elf_m68k_install_pc32 (splt, plt_info->plt0_relocs.got8,
				 (sgot->output_section->vma
				  + sgot->output_offset
				  + 8));

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize
	    = plt_info->size;
	}
    }

  /* Fill in the first three entries in the global offset table.  */
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

  return true;
}

/* Given a .data section and a .emreloc in-memory section, store
   relocation information into the .emreloc section which can be
   used at runtime to relocate the section.  This is called by the
   linker when the --embedded-relocs switch is used.  This is called
   after the add_symbols entry point has been called for all the
   objects, and before the final_link entry point is called.  */

bool
bfd_m68k_elf32_create_embedded_relocs (bfd *abfd, struct bfd_link_info *info,
				       asection *datasec, asection *relsec,
				       char **errmsg)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *p;
  bfd_size_type amt;

  BFD_ASSERT (! bfd_link_relocatable (info));

  *errmsg = NULL;

  if (datasec->reloc_count == 0)
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Get a copy of the native relocations.  */
  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, datasec, NULL, (Elf_Internal_Rela *) NULL,
		      info->keep_memory));
  if (internal_relocs == NULL)
    goto error_return;

  amt = (bfd_size_type) datasec->reloc_count * 12;
  relsec->contents = (bfd_byte *) bfd_alloc (abfd, amt);
  if (relsec->contents == NULL)
    goto error_return;

  p = relsec->contents;

  irelend = internal_relocs + datasec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++, p += 12)
    {
      asection *targetsec;

      /* We are going to write a four byte longword into the runtime
       reloc section.  The longword will be the address in the data
       section which must be relocated.  It is followed by the name
       of the target section NUL-padded or truncated to 8
       characters.  */

      /* We can only relocate absolute longword relocs at run time.  */
      if (ELF32_R_TYPE (irel->r_info) != (int) R_68K_32)
	{
	  *errmsg = _("unsupported relocation type");
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;
	}

      /* Get the target section referred to by the reloc.  */
      if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;

	  /* Read this BFD's local symbols if we haven't done so already.  */
	  if (isymbuf == NULL)
	    {
	      isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	      if (isymbuf == NULL)
		isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
						symtab_hdr->sh_info, 0,
						NULL, NULL, NULL);
	      if (isymbuf == NULL)
		goto error_return;
	    }

	  isym = isymbuf + ELF32_R_SYM (irel->r_info);
	  targetsec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

	  /* An external symbol.  */
	  indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    targetsec = h->root.u.def.section;
	  else
	    targetsec = NULL;
	}

      bfd_put_32 (abfd, irel->r_offset + datasec->output_offset, p);
      memset (p + 4, 0, 8);
      if (targetsec != NULL)
	strncpy ((char *) p + 4, targetsec->output_section->name, 8);
    }

  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (datasec)->relocs != internal_relocs)
    free (internal_relocs);
  return true;

 error_return:
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (datasec)->relocs != internal_relocs)
    free (internal_relocs);
  return false;
}

/* Set target options.  */

void
bfd_elf_m68k_set_target_options (struct bfd_link_info *info, int got_handling)
{
  struct elf_m68k_link_hash_table *htab;
  bool use_neg_got_offsets_p;
  bool allow_multigot_p;
  bool local_gp_p;

  switch (got_handling)
    {
    case 0:
      /* --got=single.  */
      local_gp_p = false;
      use_neg_got_offsets_p = false;
      allow_multigot_p = false;
      break;

    case 1:
      /* --got=negative.  */
      local_gp_p = true;
      use_neg_got_offsets_p = true;
      allow_multigot_p = false;
      break;

    case 2:
      /* --got=multigot.  */
      local_gp_p = true;
      use_neg_got_offsets_p = true;
      allow_multigot_p = true;
      break;

    default:
      BFD_ASSERT (false);
      return;
    }

  htab = elf_m68k_hash_table (info);
  if (htab != NULL)
    {
      htab->local_gp_p = local_gp_p;
      htab->use_neg_got_offsets_p = use_neg_got_offsets_p;
      htab->allow_multigot_p = allow_multigot_p;
    }
}

static enum elf_reloc_type_class
elf32_m68k_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			     const asection *rel_sec ATTRIBUTE_UNUSED,
			     const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_68K_RELATIVE:
      return reloc_class_relative;
    case R_68K_JMP_SLOT:
      return reloc_class_plt;
    case R_68K_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elf_m68k_plt_sym_val (bfd_vma i, const asection *plt,
		      const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * elf_m68k_get_plt_info (plt->owner)->size;
}

/* Support for core dump NOTE sections.  */

static bool
elf_m68k_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    default:
      return false;

    case 154:		/* Linux/m68k */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 22);

      /* pr_reg */
      offset = 70;
      size = 80;

      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf_m68k_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 124:		/* Linux/m68k elf_prpsinfo.  */
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

    if (n > 0 && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}

#define TARGET_BIG_SYM			m68k_elf32_vec
#define TARGET_BIG_NAME			"elf32-m68k"
#define ELF_MACHINE_CODE		EM_68K
#define ELF_MAXPAGESIZE			0x2000
#define elf_backend_create_dynamic_sections \
					_bfd_elf_create_dynamic_sections
#define bfd_elf32_bfd_link_hash_table_create \
					elf_m68k_link_hash_table_create
#define bfd_elf32_bfd_final_link	bfd_elf_final_link

#define elf_backend_check_relocs	elf_m68k_check_relocs
#define elf_backend_always_size_sections \
					elf_m68k_always_size_sections
#define elf_backend_adjust_dynamic_symbol \
					elf_m68k_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections \
					elf_m68k_size_dynamic_sections
#define elf_backend_final_write_processing	elf_m68k_final_write_processing
#define elf_backend_init_index_section	_bfd_elf_init_1_index_section
#define elf_backend_relocate_section	elf_m68k_relocate_section
#define elf_backend_finish_dynamic_symbol \
					elf_m68k_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					elf_m68k_finish_dynamic_sections
#define elf_backend_gc_mark_hook	elf_m68k_gc_mark_hook
#define elf_backend_copy_indirect_symbol elf_m68k_copy_indirect_symbol
#define bfd_elf32_bfd_merge_private_bfd_data \
					elf32_m68k_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags \
					elf32_m68k_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
					elf32_m68k_print_private_bfd_data
#define elf_backend_reloc_type_class	elf32_m68k_reloc_type_class
#define elf_backend_plt_sym_val		elf_m68k_plt_sym_val
#define elf_backend_object_p		elf32_m68k_object_p
#define elf_backend_grok_prstatus	elf_m68k_grok_prstatus
#define elf_backend_grok_psinfo		elf_m68k_grok_psinfo

#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_plt 1
#define elf_backend_plt_readonly 1
#define elf_backend_want_plt_sym 0
#define elf_backend_got_header_size	12
#define elf_backend_rela_normal		1
#define elf_backend_dtrel_excludes_plt	1

#define elf_backend_linux_prpsinfo32_ugid16	true

#include "elf32-target.h"
