/* MIPS-specific support for 32-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

   Most of the information added by Ian Lance Taylor, Cygnus Support,
   <ian@cygnus.com>.
   N32/64 ABI support added by Mark Mitchell, CodeSourcery, LLC.
   <mark@codesourcery.com>
   Traditional MIPS targets support added by Koundinya.K, Dansk Data
   Elektronik & Operations Research Group. <kk@ddeorg.soft.net>

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


/* This file handles MIPS ELF targets.  SGI Irix 5 uses a slightly
   different MIPS ELF from other targets.  This matters when linking.
   This file supports both, switching at runtime.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elfxx-mips.h"
#include "elf/mips.h"
#include "elf-vxworks.h"

/* Get the ECOFF swapping routines.  */
#include "coff/sym.h"
#include "coff/symconst.h"
#include "coff/internal.h"
#include "coff/ecoff.h"
#include "coff/mips.h"
#define ECOFF_SIGNED_32
#include "ecoffswap.h"

static bfd_reloc_status_type gprel32_with_gp
  (bfd *, asymbol *, arelent *, asection *, bool, void *, bfd_vma);
static bfd_reloc_status_type mips_elf_gprel32_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type mips32_64bit_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
static bool mips_info_to_howto_rel
  (bfd *, arelent *, Elf_Internal_Rela *);
static bool mips_info_to_howto_rela
  (bfd *, arelent *, Elf_Internal_Rela *);
static bool mips_elf_sym_is_global
  (bfd *, asymbol *);
static bool mips_elf32_elfsym_local_is_section
  (bfd *);
static bool mips_elf32_object_p
  (bfd *);
static bool mips_elf_is_local_label_name
  (bfd *, const char *);
static bfd_reloc_status_type mips16_gprel_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type mips_elf_final_gp
  (bfd *, asymbol *, bool, char **, bfd_vma *);
static bool mips_elf_assign_gp
  (bfd *, bfd_vma *);
static bool elf32_mips_grok_prstatus
  (bfd *, Elf_Internal_Note *);
static bool elf32_mips_grok_psinfo
  (bfd *, Elf_Internal_Note *);
static irix_compat_t elf32_mips_irix_compat
  (bfd *);

extern const bfd_target mips_elf32_be_vec;
extern const bfd_target mips_elf32_le_vec;

/* Nonzero if ABFD is using the N32 ABI.  */
#define ABI_N32_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI2) != 0)

/* Whether we are trying to be compatible with IRIX at all.  */
#define SGI_COMPAT(abfd) \
  (elf32_mips_irix_compat (abfd) != ict_none)

/* The number of local .got entries we reserve.  */
#define MIPS_RESERVED_GOTNO (2)

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)

/* The relocation table used for SHT_REL sections.  */

static reloc_howto_type elf_mips_howto_table_rel[] =
{
  /* No relocation.  */
  HOWTO (R_MIPS_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16 bit relocation.  */
  HOWTO (R_MIPS_16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_16",		/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit relocation.  */
  HOWTO (R_MIPS_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit symbol relative relocation.  */
  HOWTO (R_MIPS_REL32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_REL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 26 bit jump address.  */
  HOWTO (R_MIPS_26,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper four
				   bits must match the PC + 4.  */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_26",		/* name */
	 true,			/* partial_inplace */
	 0x03ffffff,		/* src_mask */
	 0x03ffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of symbol value.  */
  HOWTO (R_MIPS_HI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_hi16_reloc, /* special_function */
	 "R_MIPS_HI16",		/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  */
  HOWTO (R_MIPS_LO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_lo16_reloc, /* special_function */
	 "R_MIPS_LO16",		/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GP relative reference.  */
  HOWTO (R_MIPS_GPREL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf32_gprel16_reloc, /* special_function */
	 "R_MIPS_GPREL16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Reference to literal section.  */
  HOWTO (R_MIPS_LITERAL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf32_gprel16_reloc, /* special_function */
	 "R_MIPS_LITERAL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Reference to global offset table.  */
  HOWTO (R_MIPS_GOT16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_got16_reloc, /* special_function */
	 "R_MIPS_GOT16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16 bit PC relative reference.  Note that the ABI document has a typo
     and claims R_MIPS_PC16 to be not rightshifted, rendering it useless.
     We do the right thing here.  */
  HOWTO (R_MIPS_PC16,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_PC16",		/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* 16 bit call through global offset table.  */
  HOWTO (R_MIPS_CALL16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_CALL16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit GP relative reference.  */
  HOWTO (R_MIPS_GPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 mips_elf_gprel32_reloc, /* special_function */
	 "R_MIPS_GPREL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The remaining relocs are defined on Irix 5, although they are
     not defined by the ABI.  */
  EMPTY_HOWTO (13),
  EMPTY_HOWTO (14),
  EMPTY_HOWTO (15),

  /* A 5 bit shift field.  */
  HOWTO (R_MIPS_SHIFT5,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 5,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_SHIFT5",	/* name */
	 true,			/* partial_inplace */
	 0x000007c0,		/* src_mask */
	 0x000007c0,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 6 bit shift field.  */
  /* FIXME: This is not handled correctly; a special function is
     needed to put the most significant bit in the right place.  */
  HOWTO (R_MIPS_SHIFT6,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 6,			/* bitsize */
	 false,			/* pc_relative */
	 6,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_SHIFT6",	/* name */
	 true,			/* partial_inplace */
	 0x000007c4,		/* src_mask */
	 0x000007c4,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 64 bit relocation.  */
  HOWTO (R_MIPS_64,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 mips32_64bit_reloc,	/* special_function */
	 "R_MIPS_64",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Displacement in the global offset table.  */
  HOWTO (R_MIPS_GOT_DISP,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GOT_DISP",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Displacement to page pointer in the global offset table.  */
  HOWTO (R_MIPS_GOT_PAGE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GOT_PAGE",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Offset from page pointer in the global offset table.  */
  HOWTO (R_MIPS_GOT_OFST,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GOT_OFST",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_MIPS_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GOT_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_MIPS_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GOT_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 64 bit subtraction.  Used in the N32 ABI.  */
  HOWTO (R_MIPS_SUB,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_SUB",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Used to cause the linker to insert and delete instructions?  */
  EMPTY_HOWTO (R_MIPS_INSERT_A),
  EMPTY_HOWTO (R_MIPS_INSERT_B),
  EMPTY_HOWTO (R_MIPS_DELETE),

  /* Get the higher value of a 64 bit addend.  */
  HOWTO (R_MIPS_HIGHER,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_HIGHER",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Get the highest value of a 64 bit addend.  */
  HOWTO (R_MIPS_HIGHEST,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_HIGHEST",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_MIPS_CALL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_CALL_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_MIPS_CALL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_CALL_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Section displacement.  */
  HOWTO (R_MIPS_SCN_DISP,       /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_SCN_DISP",     /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (R_MIPS_REL16),
  EMPTY_HOWTO (R_MIPS_ADD_IMMEDIATE),
  EMPTY_HOWTO (R_MIPS_PJUMP),
  EMPTY_HOWTO (R_MIPS_RELGOT),

  /* Protected jump conversion.  This is an optimization hint.  No
     relocation is required for correctness.  */
  HOWTO (R_MIPS_JALR,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_JALR",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS GD/LD dynamic relocations.  */
  HOWTO (R_MIPS_TLS_DTPMOD32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_DTPMOD32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MIPS_TLS_DTPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_DTPREL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (R_MIPS_TLS_DTPMOD64),
  EMPTY_HOWTO (R_MIPS_TLS_DTPREL64),

  /* TLS general dynamic variable reference.  */
  HOWTO (R_MIPS_TLS_GD,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_GD",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic variable reference.  */
  HOWTO (R_MIPS_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_LDM",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_MIPS_TLS_DTPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_DTPREL_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_MIPS_TLS_DTPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_DTPREL_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_MIPS_TLS_GOTTPREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_GOTTPREL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS IE dynamic relocations.  */
  HOWTO (R_MIPS_TLS_TPREL32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_TPREL32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (R_MIPS_TLS_TPREL64),

  /* TLS thread pointer offset.  */
  HOWTO (R_MIPS_TLS_TPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_TPREL_HI16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_MIPS_TLS_TPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_TLS_TPREL_LO16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit relocation with no addend.  */
  HOWTO (R_MIPS_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (52),
  EMPTY_HOWTO (53),
  EMPTY_HOWTO (54),
  EMPTY_HOWTO (55),
  EMPTY_HOWTO (56),
  EMPTY_HOWTO (57),
  EMPTY_HOWTO (58),
  EMPTY_HOWTO (59),

  HOWTO (R_MIPS_PC21_S2,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_PC21_S2",	/* name */
	 true,			/* partial_inplace */
	 0x001fffff,		/* src_mask */
	 0x001fffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MIPS_PC26_S2,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_PC26_S2",	/* name */
	 true,			/* partial_inplace */
	 0x03ffffff,		/* src_mask */
	 0x03ffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MIPS_PC18_S3,	/* type */
	 3,			/* rightshift */
	 4,			/* size */
	 18,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc,   /* special_function */
	 "R_MIPS_PC18_S3",	/* name */
	 true,			/* partial_inplace */
	 0x0003ffff,		/* src_mask */
	 0x0003ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MIPS_PC19_S2,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 19,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc,   /* special_function */
	 "R_MIPS_PC19_S2",	/* name */
	 true,			/* partial_inplace */
	 0x0007ffff,		/* src_mask */
	 0x0007ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MIPS_PCHI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc,   /* special_function */
	 "R_MIPS_PCHI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MIPS_PCLO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc,   /* special_function */
	 "R_MIPS_PCLO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */
};

/* The reloc used for BFD_RELOC_CTOR when doing a 64 bit link.  This
   is a hack to make the linker think that we need 64 bit values.  */
static reloc_howto_type elf_mips_ctor64_howto =
  HOWTO (R_MIPS_64,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips32_64bit_reloc,	/* special_function */
	 "R_MIPS_64",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false);		/* pcrel_offset */

static reloc_howto_type elf_mips16_howto_table_rel[] =
{
  /* The reloc used for the mips16 jump instruction.  */
  HOWTO (R_MIPS16_26,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper four
				   bits must match the PC.  */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_26",		/* name */
	 true,			/* partial_inplace */
	 0x3ffffff,		/* src_mask */
	 0x3ffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The reloc used for the mips16 gprel instruction.  */
  HOWTO (R_MIPS16_GPREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 mips16_gprel_reloc,	/* special_function */
	 "R_MIPS16_GPREL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A MIPS16 reference to the global offset table.  */
  HOWTO (R_MIPS16_GOT16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_got16_reloc, /* special_function */
	 "R_MIPS16_GOT16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A MIPS16 call through the global offset table.  */
  HOWTO (R_MIPS16_CALL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_CALL16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 high 16 bits of symbol value.  */
  HOWTO (R_MIPS16_HI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_hi16_reloc, /* special_function */
	 "R_MIPS16_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 low 16 bits of symbol value.  */
  HOWTO (R_MIPS16_LO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_lo16_reloc, /* special_function */
	 "R_MIPS16_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS general dynamic variable reference.  */
  HOWTO (R_MIPS16_TLS_GD,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_GD",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS local dynamic variable reference.  */
  HOWTO (R_MIPS16_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_LDM",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS local dynamic offset.  */
  HOWTO (R_MIPS16_TLS_DTPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_DTPREL_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS local dynamic offset.  */
  HOWTO (R_MIPS16_TLS_DTPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_DTPREL_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS thread pointer offset.  */
  HOWTO (R_MIPS16_TLS_GOTTPREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_GOTTPREL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS thread pointer offset.  */
  HOWTO (R_MIPS16_TLS_TPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_TPREL_HI16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 TLS thread pointer offset.  */
  HOWTO (R_MIPS16_TLS_TPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_TLS_TPREL_LO16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* MIPS16 16-bit PC-relative branch offset.  */
  HOWTO (R_MIPS16_PC16_S1,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS16_PC16_S1",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */
};

static reloc_howto_type elf_micromips_howto_table_rel[] =
{
  EMPTY_HOWTO (130),
  EMPTY_HOWTO (131),
  EMPTY_HOWTO (132),

  /* 26 bit jump address.  */
  HOWTO (R_MICROMIPS_26_S1,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper four
				   bits must match the PC.  */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_26_S1",	/* name */
	 true,			/* partial_inplace */
	 0x3ffffff,		/* src_mask */
	 0x3ffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of symbol value.  */
  HOWTO (R_MICROMIPS_HI16,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_hi16_reloc, /* special_function */
	 "R_MICROMIPS_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  */
  HOWTO (R_MICROMIPS_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_lo16_reloc, /* special_function */
	 "R_MICROMIPS_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GP relative reference.  */
  HOWTO (R_MICROMIPS_GPREL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf32_gprel16_reloc, /* special_function */
	 "R_MICROMIPS_GPREL16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Reference to literal section.  */
  HOWTO (R_MICROMIPS_LITERAL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf32_gprel16_reloc, /* special_function */
	 "R_MICROMIPS_LITERAL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Reference to global offset table.  */
  HOWTO (R_MICROMIPS_GOT16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_got16_reloc, /* special_function */
	 "R_MICROMIPS_GOT16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* This is for microMIPS branches.  */
  HOWTO (R_MICROMIPS_PC7_S1,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 7,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_PC7_S1",	/* name */
	 true,			/* partial_inplace */
	 0x0000007f,		/* src_mask */
	 0x0000007f,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MICROMIPS_PC10_S1,	/* type */
	 1,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_PC10_S1",	/* name */
	 true,			/* partial_inplace */
	 0x000003ff,		/* src_mask */
	 0x000003ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_MICROMIPS_PC16_S1,	/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_PC16_S1",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* 16 bit call through global offset table.  */
  HOWTO (R_MICROMIPS_CALL16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_CALL16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (143),
  EMPTY_HOWTO (144),

  /* Displacement in the global offset table.  */
  HOWTO (R_MICROMIPS_GOT_DISP,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_GOT_DISP",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Displacement to page pointer in the global offset table.  */
  HOWTO (R_MICROMIPS_GOT_PAGE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_GOT_PAGE",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Offset from page pointer in the global offset table.  */
  HOWTO (R_MICROMIPS_GOT_OFST,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_GOT_OFST",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_MICROMIPS_GOT_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_GOT_HI16",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_MICROMIPS_GOT_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_GOT_LO16",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 64 bit subtraction.  Used in the N32 ABI.  */
  HOWTO (R_MICROMIPS_SUB,	/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_SUB",	/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Get the higher value of a 64 bit addend.  */
  HOWTO (R_MICROMIPS_HIGHER,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_HIGHER",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Get the highest value of a 64 bit addend.  */
  HOWTO (R_MICROMIPS_HIGHEST,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_HIGHEST",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* High 16 bits of displacement in global offset table.  */
  HOWTO (R_MICROMIPS_CALL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_CALL_HI16",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of displacement in global offset table.  */
  HOWTO (R_MICROMIPS_CALL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_CALL_LO16",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Section displacement.  */
  HOWTO (R_MICROMIPS_SCN_DISP,  /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_SCN_DISP",/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Protected jump conversion.  This is an optimization hint.  No
     relocation is required for correctness.  */
  HOWTO (R_MICROMIPS_JALR,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_JALR",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  Note that the high 16 bits of symbol values
     must be zero.  This is used for relaxation.  */
  HOWTO (R_MICROMIPS_HI0_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_HI0_LO16",/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (158),
  EMPTY_HOWTO (159),
  EMPTY_HOWTO (160),
  EMPTY_HOWTO (161),

  /* TLS general dynamic variable reference.  */
  HOWTO (R_MICROMIPS_TLS_GD,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_GD",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic variable reference.  */
  HOWTO (R_MICROMIPS_TLS_LDM,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_LDM",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_MICROMIPS_TLS_DTPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_DTPREL_HI16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS local dynamic offset.  */
  HOWTO (R_MICROMIPS_TLS_DTPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_DTPREL_LO16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_MICROMIPS_TLS_GOTTPREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_GOTTPREL",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (167),
  EMPTY_HOWTO (168),

  /* TLS thread pointer offset.  */
  HOWTO (R_MICROMIPS_TLS_TPREL_HI16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_TPREL_HI16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* TLS thread pointer offset.  */
  HOWTO (R_MICROMIPS_TLS_TPREL_LO16,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_TLS_TPREL_LO16", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (171),

  /* GP- and PC-relative relocations.  */
  HOWTO (R_MICROMIPS_GPREL7_S2,	/* type */
	 2,			/* rightshift */
	 2,			/* size */
	 7,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf32_gprel16_reloc, /* special_function */
	 "R_MICROMIPS_GPREL7_S2",	/* name */
	 true,			/* partial_inplace */
	 0x0000007f,		/* src_mask */
	 0x0000007f,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_MICROMIPS_PC23_S2,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 23,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MICROMIPS_PC23_S2",	/* name */
	 true,			/* partial_inplace */
	 0x007fffff,		/* src_mask */
	 0x007fffff,		/* dst_mask */
	 true),			/* pcrel_offset */
};

/* 16 bit offset for pc-relative branches.  */
static reloc_howto_type elf_mips_gnu_rel16_s2 =
  HOWTO (R_MIPS_GNU_REL16_S2,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_GNU_REL16_S2",	/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 true);			/* pcrel_offset */

/* 32 bit pc-relative.  This was a GNU extension used by embedded-PIC.
   It was co-opted by mips-linux for exception-handling data.  GCC stopped
   using it in May, 2004, then started using it again for compact unwind
   tables.  */
static reloc_howto_type elf_mips_gnu_pcrel32 =
  HOWTO (R_MIPS_PC32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_PC32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true);			/* pcrel_offset */

/* GNU extension to record C++ vtable hierarchy */
static reloc_howto_type elf_mips_gnu_vtinherit_howto =
  HOWTO (R_MIPS_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_MIPS_GNU_VTINHERIT", /* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false);		/* pcrel_offset */

/* GNU extension to record C++ vtable member usage */
static reloc_howto_type elf_mips_gnu_vtentry_howto =
  HOWTO (R_MIPS_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_MIPS_GNU_VTENTRY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false);		/* pcrel_offset */

/* Originally a VxWorks extension, but now used for other systems too.  */
static reloc_howto_type elf_mips_copy_howto =
  HOWTO (R_MIPS_COPY,		/* type */
	 0,			/* rightshift */
	 0,			/* this one is variable size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_COPY",		/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x0,			/* dst_mask */
	 false);		/* pcrel_offset */

/* Originally a VxWorks extension, but now used for other systems too.  */
static reloc_howto_type elf_mips_jump_slot_howto =
  HOWTO (R_MIPS_JUMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_JUMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0x0,			/* src_mask */
	 0x0,			/* dst_mask */
	 false);		/* pcrel_offset */

/* Used in EH tables.  */
static reloc_howto_type elf_mips_eh_howto =
  HOWTO (R_MIPS_EH,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 _bfd_mips_elf_generic_reloc, /* special_function */
	 "R_MIPS_EH",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false);		/* pcrel_offset */

/* Set the GP value for OUTPUT_BFD.  Returns FALSE if this is a
   dangerous relocation.  */

static bool
mips_elf_assign_gp (bfd *output_bfd, bfd_vma *pgp)
{
  unsigned int count;
  asymbol **sym;
  unsigned int i;

  /* If we've already figured out what GP will be, just return it.  */
  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp)
    return true;

  count = bfd_get_symcount (output_bfd);
  sym = bfd_get_outsymbols (output_bfd);

  /* The linker script will have created a symbol named `_gp' with the
     appropriate value.  */
  if (sym == NULL)
    i = count;
  else
    {
      for (i = 0; i < count; i++, sym++)
	{
	  register const char *name;

	  name = bfd_asymbol_name (*sym);
	  if (*name == '_' && strcmp (name, "_gp") == 0)
	    {
	      *pgp = bfd_asymbol_value (*sym);
	      _bfd_set_gp_value (output_bfd, *pgp);
	      break;
	    }
	}
    }

  if (i >= count)
    {
      /* Only get the error once.  */
      *pgp = 4;
      _bfd_set_gp_value (output_bfd, *pgp);
      return false;
    }

  return true;
}

/* We have to figure out the gp value, so that we can adjust the
   symbol value correctly.  We look up the symbol _gp in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */

static bfd_reloc_status_type
mips_elf_final_gp (bfd *output_bfd, asymbol *symbol, bool relocatable,
		   char **error_message, bfd_vma *pgp)
{
  if (output_bfd == NULL)
    {
      *pgp = 0;
      return bfd_reloc_undefined;
    }

  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp == 0
      && (! relocatable
	  || (symbol->flags & BSF_SECTION_SYM) != 0))
    {
      if (relocatable)
	{
	  /* Make up a value.  */
	  *pgp = symbol->section->output_section->vma /*+ 0x4000*/;
	  _bfd_set_gp_value (output_bfd, *pgp);
	}
      else if (!mips_elf_assign_gp (output_bfd, pgp))
	{
	  *error_message =
	    (char *) _("GP relative relocation when _gp not defined");
	  return bfd_reloc_dangerous;
	}
    }

  return bfd_reloc_ok;
}

/* Do a R_MIPS_GPREL16 relocation.  This is a 16 bit value which must
   become the offset from the gp register.  This function also handles
   R_MIPS_LITERAL relocations, although those can be handled more
   cleverly because the entries in the .lit8 and .lit4 sections can be
   merged.  */

bfd_reloc_status_type
_bfd_mips_elf32_gprel16_reloc (bfd *abfd, arelent *reloc_entry,
			       asymbol *symbol, void *data,
			       asection *input_section, bfd *output_bfd,
			       char **error_message)
{
  bool relocatable;
  bfd_reloc_status_type ret;
  bfd_byte *location;
  bfd_vma gp;

  /* R_MIPS_LITERAL/R_MICROMIPS_LITERAL relocations are defined for local
     symbols only.  */
  if (literal_reloc_p (reloc_entry->howto->type)
      && output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      *error_message = (char *)
	_("literal relocation occurs for an external symbol");
      return bfd_reloc_outofrange;
    }

  if (output_bfd != NULL)
    relocatable = true;
  else
    {
      relocatable = false;
      output_bfd = input_section->output_section->owner;
    }

  ret = mips_elf_final_gp (output_bfd, symbol, relocatable, error_message,
			   &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  if (!_bfd_mips_reloc_offset_in_range (abfd, input_section, reloc_entry,
					check_shuffle))
    return bfd_reloc_outofrange;

  location = (bfd_byte *) data + reloc_entry->address;
  _bfd_mips_elf_reloc_unshuffle (abfd, reloc_entry->howto->type, false,
				 location);
  ret = _bfd_mips_elf_gprel16_with_gp (abfd, symbol, reloc_entry,
				       input_section, relocatable,
				       data, gp);
  _bfd_mips_elf_reloc_shuffle (abfd, reloc_entry->howto->type, !relocatable,
			       location);

  return ret;
}

/* Do a R_MIPS_GPREL32 relocation.  This is a 32 bit value which must
   become the offset from the gp register.  */

static bfd_reloc_status_type
mips_elf_gprel32_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			void *data, asection *input_section, bfd *output_bfd,
			char **error_message)
{
  bool relocatable;
  bfd_reloc_status_type ret;
  bfd_vma gp;

  /* R_MIPS_GPREL32 relocations are defined for local symbols only.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      *error_message = (char *)
	_("32bits gp relative relocation occurs for an external symbol");
      return bfd_reloc_outofrange;
    }

  if (output_bfd != NULL)
    relocatable = true;
  else
    {
      relocatable = false;
      output_bfd = input_section->output_section->owner;
    }

  ret = mips_elf_final_gp (output_bfd, symbol, relocatable,
			   error_message, &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  return gprel32_with_gp (abfd, symbol, reloc_entry, input_section,
			  relocatable, data, gp);
}

static bfd_reloc_status_type
gprel32_with_gp (bfd *abfd, asymbol *symbol, arelent *reloc_entry,
		 asection *input_section, bool relocatable,
		 void *data, bfd_vma gp)
{
  bfd_vma relocation;
  bfd_vma val;

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  if (symbol->section->output_section != NULL)
    {
      relocation += symbol->section->output_section->vma;
      relocation += symbol->section->output_offset;
    }

  if (!_bfd_mips_reloc_offset_in_range (abfd, input_section, reloc_entry,
					check_inplace))
    return bfd_reloc_outofrange;

  /* Set val to the offset into the section or symbol.  */
  val = reloc_entry->addend;

  if (reloc_entry->howto->partial_inplace)
    val += bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);

  /* Adjust val for the final section location and GP value.  If we
     are producing relocatable output, we don't want to do this for
     an external symbol.  */
  if (! relocatable
      || (symbol->flags & BSF_SECTION_SYM) != 0)
    val += relocation - gp;

  if (reloc_entry->howto->partial_inplace)
    bfd_put_32 (abfd, val, (bfd_byte *) data + reloc_entry->address);
  else
    reloc_entry->addend = val;

  if (relocatable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

/* Handle a 64 bit reloc in a 32 bit MIPS ELF file.  These are
   generated when addresses are 64 bits.  The upper 32 bits are a simple
   sign extension.  */

static bfd_reloc_status_type
mips32_64bit_reloc (bfd *abfd, arelent *reloc_entry,
		    asymbol *symbol ATTRIBUTE_UNUSED,
		    void *data, asection *input_section,
		    bfd *output_bfd, char **error_message)
{
  bfd_reloc_status_type r;
  arelent reloc32;
  unsigned long val;
  bfd_size_type addr;

  /* Do a normal 32 bit relocation on the lower 32 bits.  */
  reloc32 = *reloc_entry;
  if (bfd_big_endian (abfd))
    reloc32.address += 4;
  reloc32.howto = &elf_mips_howto_table_rel[R_MIPS_32];
  r = bfd_perform_relocation (abfd, &reloc32, data, input_section,
			      output_bfd, error_message);

  /* Sign extend into the upper 32 bits.  */
  val = bfd_get_32 (abfd, (bfd_byte *) data + reloc32.address);
  if ((val & 0x80000000) != 0)
    val = 0xffffffff;
  else
    val = 0;
  addr = reloc_entry->address;
  if (bfd_little_endian (abfd))
    addr += 4;
  bfd_put_32 (abfd, val, (bfd_byte *) data + addr);

  return r;
}

/* Handle a mips16 GP relative reloc.  */

static bfd_reloc_status_type
mips16_gprel_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		    void *data, asection *input_section, bfd *output_bfd,
		    char **error_message)
{
  bool relocatable;
  bfd_reloc_status_type ret;
  bfd_byte *location;
  bfd_vma gp;

  /* If we're relocating, and this is an external symbol, we don't want
     to change anything.  */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (symbol->flags & BSF_LOCAL) != 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    relocatable = true;
  else
    {
      relocatable = false;
      output_bfd = input_section->output_section->owner;
    }

  ret = mips_elf_final_gp (output_bfd, symbol, relocatable, error_message,
			   &gp);
  if (ret != bfd_reloc_ok)
    return ret;

  if (!_bfd_mips_reloc_offset_in_range (abfd, input_section, reloc_entry,
					check_shuffle))
    return bfd_reloc_outofrange;

  location = (bfd_byte *) data + reloc_entry->address;
  _bfd_mips_elf_reloc_unshuffle (abfd, reloc_entry->howto->type, false,
				 location);
  ret = _bfd_mips_elf_gprel16_with_gp (abfd, symbol, reloc_entry,
				       input_section, relocatable,
				       data, gp);
  _bfd_mips_elf_reloc_shuffle (abfd, reloc_entry->howto->type, !relocatable,
			       location);

  return ret;
}

/* A mapping from BFD reloc types to MIPS ELF reloc types.  */

struct elf_reloc_map {
  bfd_reloc_code_real_type bfd_val;
  enum elf_mips_reloc_type elf_val;
};

static const struct elf_reloc_map mips_reloc_map[] =
{
  { BFD_RELOC_NONE, R_MIPS_NONE },
  { BFD_RELOC_MIPS_16, R_MIPS_16 },
  { BFD_RELOC_32, R_MIPS_32 },
  /* There is no BFD reloc for R_MIPS_REL32.  */
  { BFD_RELOC_64, R_MIPS_64 },
  { BFD_RELOC_MIPS_JMP, R_MIPS_26 },
  { BFD_RELOC_HI16_S, R_MIPS_HI16 },
  { BFD_RELOC_LO16, R_MIPS_LO16 },
  { BFD_RELOC_GPREL16, R_MIPS_GPREL16 },
  { BFD_RELOC_MIPS_LITERAL, R_MIPS_LITERAL },
  { BFD_RELOC_MIPS_GOT16, R_MIPS_GOT16 },
  { BFD_RELOC_16_PCREL_S2, R_MIPS_PC16 },
  { BFD_RELOC_MIPS_CALL16, R_MIPS_CALL16 },
  { BFD_RELOC_GPREL32, R_MIPS_GPREL32 },
  { BFD_RELOC_MIPS_GOT_HI16, R_MIPS_GOT_HI16 },
  { BFD_RELOC_MIPS_GOT_LO16, R_MIPS_GOT_LO16 },
  { BFD_RELOC_MIPS_CALL_HI16, R_MIPS_CALL_HI16 },
  { BFD_RELOC_MIPS_CALL_LO16, R_MIPS_CALL_LO16 },
  { BFD_RELOC_MIPS_SUB, R_MIPS_SUB },
  { BFD_RELOC_MIPS_GOT_PAGE, R_MIPS_GOT_PAGE },
  { BFD_RELOC_MIPS_GOT_OFST, R_MIPS_GOT_OFST },
  { BFD_RELOC_MIPS_GOT_DISP, R_MIPS_GOT_DISP },
  { BFD_RELOC_MIPS_JALR, R_MIPS_JALR },
  { BFD_RELOC_MIPS_TLS_DTPMOD32, R_MIPS_TLS_DTPMOD32 },
  { BFD_RELOC_MIPS_TLS_DTPREL32, R_MIPS_TLS_DTPREL32 },
  { BFD_RELOC_MIPS_TLS_DTPMOD64, R_MIPS_TLS_DTPMOD64 },
  { BFD_RELOC_MIPS_TLS_DTPREL64, R_MIPS_TLS_DTPREL64 },
  { BFD_RELOC_MIPS_TLS_GD, R_MIPS_TLS_GD },
  { BFD_RELOC_MIPS_TLS_LDM, R_MIPS_TLS_LDM },
  { BFD_RELOC_MIPS_TLS_DTPREL_HI16, R_MIPS_TLS_DTPREL_HI16 },
  { BFD_RELOC_MIPS_TLS_DTPREL_LO16, R_MIPS_TLS_DTPREL_LO16 },
  { BFD_RELOC_MIPS_TLS_GOTTPREL, R_MIPS_TLS_GOTTPREL },
  { BFD_RELOC_MIPS_TLS_TPREL32, R_MIPS_TLS_TPREL32 },
  { BFD_RELOC_MIPS_TLS_TPREL64, R_MIPS_TLS_TPREL64 },
  { BFD_RELOC_MIPS_TLS_TPREL_HI16, R_MIPS_TLS_TPREL_HI16 },
  { BFD_RELOC_MIPS_TLS_TPREL_LO16, R_MIPS_TLS_TPREL_LO16 },
  { BFD_RELOC_MIPS_21_PCREL_S2, R_MIPS_PC21_S2 },
  { BFD_RELOC_MIPS_26_PCREL_S2, R_MIPS_PC26_S2 },
  { BFD_RELOC_MIPS_18_PCREL_S3, R_MIPS_PC18_S3 },
  { BFD_RELOC_MIPS_19_PCREL_S2, R_MIPS_PC19_S2 },
  { BFD_RELOC_HI16_S_PCREL, R_MIPS_PCHI16 },
  { BFD_RELOC_LO16_PCREL, R_MIPS_PCLO16 }
};

static const struct elf_reloc_map mips16_reloc_map[] =
{
  { BFD_RELOC_MIPS16_JMP, R_MIPS16_26 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_GPREL, R_MIPS16_GPREL - R_MIPS16_min },
  { BFD_RELOC_MIPS16_GOT16, R_MIPS16_GOT16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_CALL16, R_MIPS16_CALL16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_HI16_S, R_MIPS16_HI16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_LO16, R_MIPS16_LO16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_GD, R_MIPS16_TLS_GD - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_LDM, R_MIPS16_TLS_LDM - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_DTPREL_HI16,
    R_MIPS16_TLS_DTPREL_HI16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_DTPREL_LO16,
    R_MIPS16_TLS_DTPREL_LO16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_GOTTPREL, R_MIPS16_TLS_GOTTPREL - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_TPREL_HI16, R_MIPS16_TLS_TPREL_HI16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_TLS_TPREL_LO16, R_MIPS16_TLS_TPREL_LO16 - R_MIPS16_min },
  { BFD_RELOC_MIPS16_16_PCREL_S1, R_MIPS16_PC16_S1 - R_MIPS16_min }
};

static const struct elf_reloc_map micromips_reloc_map[] =
{
  { BFD_RELOC_MICROMIPS_JMP, R_MICROMIPS_26_S1 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_HI16_S, R_MICROMIPS_HI16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_LO16, R_MICROMIPS_LO16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GPREL16, R_MICROMIPS_GPREL16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_LITERAL, R_MICROMIPS_LITERAL - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT16, R_MICROMIPS_GOT16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_7_PCREL_S1, R_MICROMIPS_PC7_S1 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_10_PCREL_S1, R_MICROMIPS_PC10_S1 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_16_PCREL_S1, R_MICROMIPS_PC16_S1 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_CALL16, R_MICROMIPS_CALL16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT_DISP, R_MICROMIPS_GOT_DISP - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT_PAGE, R_MICROMIPS_GOT_PAGE - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT_OFST, R_MICROMIPS_GOT_OFST - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT_HI16, R_MICROMIPS_GOT_HI16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_GOT_LO16, R_MICROMIPS_GOT_LO16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_SUB, R_MICROMIPS_SUB - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_HIGHER, R_MICROMIPS_HIGHER - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_HIGHEST, R_MICROMIPS_HIGHEST - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_CALL_HI16, R_MICROMIPS_CALL_HI16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_CALL_LO16, R_MICROMIPS_CALL_LO16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_SCN_DISP, R_MICROMIPS_SCN_DISP - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_JALR, R_MICROMIPS_JALR - R_MICROMIPS_min },
  /* There is no BFD reloc for R_MICROMIPS_HI0_LO16.  */
  { BFD_RELOC_MICROMIPS_TLS_GD, R_MICROMIPS_TLS_GD - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_LDM, R_MICROMIPS_TLS_LDM - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_DTPREL_HI16,
    R_MICROMIPS_TLS_DTPREL_HI16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_DTPREL_LO16,
    R_MICROMIPS_TLS_DTPREL_LO16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_GOTTPREL,
    R_MICROMIPS_TLS_GOTTPREL - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_TPREL_HI16,
    R_MICROMIPS_TLS_TPREL_HI16 - R_MICROMIPS_min },
  { BFD_RELOC_MICROMIPS_TLS_TPREL_LO16,
    R_MICROMIPS_TLS_TPREL_LO16 - R_MICROMIPS_min },
  /* There is no BFD reloc for R_MICROMIPS_GPREL7_S2.  */
  /* There is no BFD reloc for R_MICROMIPS_PC23_S2.  */
};

/* Given a BFD reloc type, return a howto structure.  */

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  unsigned int i;
  reloc_howto_type *howto_table = elf_mips_howto_table_rel;
  reloc_howto_type *howto16_table = elf_mips16_howto_table_rel;
  reloc_howto_type *howto_micromips_table = elf_micromips_howto_table_rel;

  for (i = 0; i < sizeof (mips_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (mips_reloc_map[i].bfd_val == code)
	return &howto_table[(int) mips_reloc_map[i].elf_val];
    }

  for (i = 0; i < sizeof (mips16_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (mips16_reloc_map[i].bfd_val == code)
	return &howto16_table[(int) mips16_reloc_map[i].elf_val];
    }

  for (i = 0; i < sizeof (micromips_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (micromips_reloc_map[i].bfd_val == code)
	return &howto_micromips_table[(int) micromips_reloc_map[i].elf_val];
    }

  switch (code)
    {
    default:
      bfd_set_error (bfd_error_bad_value);
      return NULL;

    case BFD_RELOC_CTOR:
      /* We need to handle BFD_RELOC_CTOR specially.
	 Select the right relocation (R_MIPS_32 or R_MIPS_64) based on the
	 size of addresses of the ABI.  */
      if ((elf_elfheader (abfd)->e_flags & (E_MIPS_ABI_O64
					    | E_MIPS_ABI_EABI64)) != 0)
	return &elf_mips_ctor64_howto;
      else
	return &howto_table[(int) R_MIPS_32];

    case BFD_RELOC_VTABLE_INHERIT:
      return &elf_mips_gnu_vtinherit_howto;
    case BFD_RELOC_VTABLE_ENTRY:
      return &elf_mips_gnu_vtentry_howto;
    case BFD_RELOC_32_PCREL:
      return &elf_mips_gnu_pcrel32;
    case BFD_RELOC_MIPS_COPY:
      return &elf_mips_copy_howto;
    case BFD_RELOC_MIPS_JUMP_SLOT:
      return &elf_mips_jump_slot_howto;
    case BFD_RELOC_MIPS_EH:
      return &elf_mips_eh_howto;
    }
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (elf_mips_howto_table_rel)
	    / sizeof (elf_mips_howto_table_rel[0]));
       i++)
    if (elf_mips_howto_table_rel[i].name != NULL
	&& strcasecmp (elf_mips_howto_table_rel[i].name, r_name) == 0)
      return &elf_mips_howto_table_rel[i];

  for (i = 0;
       i < (sizeof (elf_mips16_howto_table_rel)
	    / sizeof (elf_mips16_howto_table_rel[0]));
       i++)
    if (elf_mips16_howto_table_rel[i].name != NULL
	&& strcasecmp (elf_mips16_howto_table_rel[i].name, r_name) == 0)
      return &elf_mips16_howto_table_rel[i];

  for (i = 0;
       i < (sizeof (elf_micromips_howto_table_rel)
	    / sizeof (elf_micromips_howto_table_rel[0]));
       i++)
    if (elf_micromips_howto_table_rel[i].name != NULL
	&& strcasecmp (elf_micromips_howto_table_rel[i].name, r_name) == 0)
      return &elf_micromips_howto_table_rel[i];

  if (strcasecmp (elf_mips_gnu_pcrel32.name, r_name) == 0)
    return &elf_mips_gnu_pcrel32;
  if (strcasecmp (elf_mips_gnu_rel16_s2.name, r_name) == 0)
    return &elf_mips_gnu_rel16_s2;
  if (strcasecmp (elf_mips_gnu_vtinherit_howto.name, r_name) == 0)
    return &elf_mips_gnu_vtinherit_howto;
  if (strcasecmp (elf_mips_gnu_vtentry_howto.name, r_name) == 0)
    return &elf_mips_gnu_vtentry_howto;
  if (strcasecmp (elf_mips_copy_howto.name, r_name) == 0)
    return &elf_mips_copy_howto;
  if (strcasecmp (elf_mips_jump_slot_howto.name, r_name) == 0)
    return &elf_mips_jump_slot_howto;
  if (strcasecmp (elf_mips_eh_howto.name, r_name) == 0)
    return &elf_mips_eh_howto;

  return NULL;
}

/* Given a MIPS Elf_Internal_Rel, fill in an arelent structure.  */

static reloc_howto_type *
mips_elf32_rtype_to_howto (bfd *abfd,
			   unsigned int r_type,
			   bool rela_p ATTRIBUTE_UNUSED)
{
  reloc_howto_type *howto = NULL;

  switch (r_type)
    {
    case R_MIPS_GNU_VTINHERIT:
      return &elf_mips_gnu_vtinherit_howto;
    case R_MIPS_GNU_VTENTRY:
      return &elf_mips_gnu_vtentry_howto;
    case R_MIPS_GNU_REL16_S2:
      return &elf_mips_gnu_rel16_s2;
    case R_MIPS_PC32:
      return &elf_mips_gnu_pcrel32;
    case R_MIPS_COPY:
      return &elf_mips_copy_howto;
    case R_MIPS_JUMP_SLOT:
      return &elf_mips_jump_slot_howto;
    case R_MIPS_EH:
      return &elf_mips_eh_howto;
    default:
      if (r_type >= R_MICROMIPS_min && r_type < R_MICROMIPS_max)
	howto = &elf_micromips_howto_table_rel[r_type - R_MICROMIPS_min];
      if (r_type >= R_MIPS16_min && r_type < R_MIPS16_max)
	howto = &elf_mips16_howto_table_rel[r_type - R_MIPS16_min];
      if (r_type < R_MIPS_max)
	howto = &elf_mips_howto_table_rel[r_type];
      if (howto != NULL && howto->name != NULL)
	return howto;

      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
}

/* Given a MIPS Elf_Internal_Rel, fill in an arelent structure.  */

static bool
mips_info_to_howto_rel (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = mips_elf32_rtype_to_howto (abfd, r_type, false);

  if (cache_ptr->howto == NULL)
    return false;

  /* The addend for a GPREL16 or LITERAL relocation comes from the GP
     value for the object file.  We get the addend now, rather than
     when we do the relocation, because the symbol manipulations done
     by the linker may cause us to lose track of the input BFD.  */
  if (((*cache_ptr->sym_ptr_ptr)->flags & BSF_SECTION_SYM) != 0
      && (gprel16_reloc_p (r_type) || literal_reloc_p (r_type)))
    cache_ptr->addend = elf_gp (abfd);

  return true;
}

/* Given a MIPS Elf_Internal_Rela, fill in an arelent structure.  */

static bool
mips_info_to_howto_rela (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  return mips_info_to_howto_rel (abfd, cache_ptr, dst);

  /* If we ever need to do any extra processing with dst->r_addend
     (the field omitted in an Elf_Internal_Rel) we can do it here.  */
}

/* Determine whether a symbol is global for the purposes of splitting
   the symbol table into global symbols and local symbols.  At least
   on Irix 5, this split must be between section symbols and all other
   symbols.  On most ELF targets the split is between static symbols
   and externally visible symbols.  */

static bool
mips_elf_sym_is_global (bfd *abfd ATTRIBUTE_UNUSED, asymbol *sym)
{
  if (SGI_COMPAT (abfd))
    return (sym->flags & BSF_SECTION_SYM) == 0;
  else
    return ((sym->flags & (BSF_GLOBAL | BSF_WEAK | BSF_GNU_UNIQUE)) != 0
	    || bfd_is_und_section (bfd_asymbol_section (sym))
	    || bfd_is_com_section (bfd_asymbol_section (sym)));
}

/* Likewise, return TRUE if the symbol table split overall must be
   between section symbols and all other symbols.  */
static bool
mips_elf32_elfsym_local_is_section (bfd *abfd)
{
  return SGI_COMPAT (abfd);
}

/* Set the right machine number for a MIPS ELF file.  */

static bool
mips_elf32_object_p (bfd *abfd)
{
  unsigned long mach;

  if (ABI_N32_P (abfd))
    return false;

  /* Irix 5 and 6 are broken.  Object file symbol tables are not always
     sorted correctly such that local symbols precede global symbols,
     and the sh_info field in the symbol table is not always right.  */
  if (SGI_COMPAT (abfd))
    elf_bad_symtab (abfd) = true;

  mach = _bfd_elf_mips_mach (elf_elfheader (abfd)->e_flags);
  bfd_default_set_arch_mach (abfd, bfd_arch_mips, mach);
  return true;
}

/* MIPS ELF local labels start with '$', not 'L'.  */

static bool
mips_elf_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == '$')
    return true;

  /* On Irix 6, the labels go back to starting with '.', so we accept
     the generic ELF local label syntax as well.  */
  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Support for core dump NOTE sections.  */
static bool
elf32_mips_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
      default:
	return false;

      case 256:		/* Linux/MIPS */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

	/* pr_reg */
	offset = 72;
	size = 180;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf32_mips_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return false;

      case 128:		/* Linux/MIPS elf_prpsinfo */
	elf_tdata (abfd)->core->pid
	 = bfd_get_32 (abfd, note->descdata + 16);
	elf_tdata (abfd)->core->program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 32, 16);
	elf_tdata (abfd)->core->command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 48, 80);
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

/* Write Linux core PRSTATUS note into core file.  */

static char *
elf32_mips_write_core_note (bfd *abfd, char *buf, int *bufsiz, int note_type,
			     ...)
{
  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      BFD_FAIL ();
      return NULL;

    case NT_PRSTATUS:
      {
	char data[256];
	va_list ap;
	long pid;
	int cursig;
	const void *greg;

	va_start (ap, note_type);
	memset (data, 0, 72);
	pid = va_arg (ap, long);
	bfd_put_32 (abfd, pid, data + 24);
	cursig = va_arg (ap, int);
	bfd_put_16 (abfd, cursig, data + 12);
	greg = va_arg (ap, const void *);
	memcpy (data + 72, greg, 180);
	memset (data + 252, 0, 4);
	va_end (ap);
	return elfcore_write_note (abfd, buf, bufsiz,
				   "CORE", note_type, data, sizeof (data));
      }
    }
}

/* Remove the magic _gp_disp symbol from the symbol tables.  */

static bool
elf32_mips_fixup_symbol (struct bfd_link_info *info,
			 struct elf_link_hash_entry *h)
{
  if (strcmp (h->root.root.string, "_gp_disp") == 0)
    _bfd_elf_link_hash_hide_symbol (info, h, true);
  return true;
}

/* Depending on the target vector we generate some version of Irix
   executables or "normal" MIPS ELF ABI executables.  */
static irix_compat_t
elf32_mips_irix_compat (bfd *abfd)
{
  if ((abfd->xvec == &mips_elf32_be_vec)
      || (abfd->xvec == &mips_elf32_le_vec))
    return ict_irix5;
  else
    return ict_none;
}

/* ECOFF swapping routines.  These are used when dealing with the
   .mdebug section, which is in the ECOFF debugging format.  */
static const struct ecoff_debug_swap mips_elf32_ecoff_debug_swap = {
  /* Symbol table magic number.  */
  magicSym,
  /* Alignment of debugging information.  E.g., 4.  */
  4,
  /* Sizes of external symbolic information.  */
  sizeof (struct hdr_ext),
  sizeof (struct dnr_ext),
  sizeof (struct pdr_ext),
  sizeof (struct sym_ext),
  sizeof (struct opt_ext),
  sizeof (struct fdr_ext),
  sizeof (struct rfd_ext),
  sizeof (struct ext_ext),
  /* Functions to swap in external symbolic data.  */
  ecoff_swap_hdr_in,
  ecoff_swap_dnr_in,
  ecoff_swap_pdr_in,
  ecoff_swap_sym_in,
  ecoff_swap_opt_in,
  ecoff_swap_fdr_in,
  ecoff_swap_rfd_in,
  ecoff_swap_ext_in,
  _bfd_ecoff_swap_tir_in,
  _bfd_ecoff_swap_rndx_in,
  /* Functions to swap out external symbolic data.  */
  ecoff_swap_hdr_out,
  ecoff_swap_dnr_out,
  ecoff_swap_pdr_out,
  ecoff_swap_sym_out,
  ecoff_swap_opt_out,
  ecoff_swap_fdr_out,
  ecoff_swap_rfd_out,
  ecoff_swap_ext_out,
  _bfd_ecoff_swap_tir_out,
  _bfd_ecoff_swap_rndx_out,
  /* Function to read in symbolic data.  */
  _bfd_mips_elf_read_ecoff_info
};

#define ELF_ARCH			bfd_arch_mips
#define ELF_TARGET_ID			MIPS_ELF_DATA
#define ELF_MACHINE_CODE		EM_MIPS

#define elf_backend_collect		true
#define elf_backend_type_change_ok	true
#define elf_backend_can_gc_sections	true
#define elf_backend_gc_mark_extra_sections \
					_bfd_mips_elf_gc_mark_extra_sections
#define elf_info_to_howto		mips_info_to_howto_rela
#define elf_info_to_howto_rel		mips_info_to_howto_rel
#define elf_backend_sym_is_global	mips_elf_sym_is_global
#define elf_backend_object_p		mips_elf32_object_p
#define elf_backend_symbol_processing	_bfd_mips_elf_symbol_processing
#define elf_backend_section_processing	_bfd_mips_elf_section_processing
#define elf_backend_section_from_shdr	_bfd_mips_elf_section_from_shdr
#define elf_backend_fake_sections	_bfd_mips_elf_fake_sections
#define elf_backend_section_from_bfd_section \
					_bfd_mips_elf_section_from_bfd_section
#define elf_backend_add_symbol_hook	_bfd_mips_elf_add_symbol_hook
#define elf_backend_link_output_symbol_hook \
					_bfd_mips_elf_link_output_symbol_hook
#define elf_backend_create_dynamic_sections \
					_bfd_mips_elf_create_dynamic_sections
#define elf_backend_check_relocs	_bfd_mips_elf_check_relocs
#define elf_backend_merge_symbol_attribute \
					_bfd_mips_elf_merge_symbol_attribute
#define elf_backend_get_target_dtag	_bfd_mips_elf_get_target_dtag
#define elf_backend_adjust_dynamic_symbol \
					_bfd_mips_elf_adjust_dynamic_symbol
#define elf_backend_always_size_sections \
					_bfd_mips_elf_always_size_sections
#define elf_backend_size_dynamic_sections \
					_bfd_mips_elf_size_dynamic_sections
#define elf_backend_init_index_section	_bfd_elf_init_1_index_section
#define elf_backend_relocate_section	_bfd_mips_elf_relocate_section
#define elf_backend_finish_dynamic_symbol \
					_bfd_mips_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					_bfd_mips_elf_finish_dynamic_sections
#define elf_backend_final_write_processing \
					_bfd_mips_elf_final_write_processing
#define elf_backend_additional_program_headers \
					_bfd_mips_elf_additional_program_headers
#define elf_backend_modify_segment_map	_bfd_mips_elf_modify_segment_map
#define elf_backend_gc_mark_hook	_bfd_mips_elf_gc_mark_hook
#define elf_backend_copy_indirect_symbol \
					_bfd_mips_elf_copy_indirect_symbol
#define elf_backend_hide_symbol		_bfd_mips_elf_hide_symbol
#define elf_backend_fixup_symbol	elf32_mips_fixup_symbol
#define elf_backend_grok_prstatus	elf32_mips_grok_prstatus
#define elf_backend_grok_psinfo		elf32_mips_grok_psinfo
#define elf_backend_ecoff_debug_swap	&mips_elf32_ecoff_debug_swap

#define elf_backend_got_header_size	(4 * MIPS_RESERVED_GOTNO)
#define elf_backend_want_dynrelro	1
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	0
#define elf_backend_default_use_rela_p	0
#define elf_backend_sign_extend_vma	true
#define elf_backend_plt_readonly	1

#define elf_backend_discard_info	_bfd_mips_elf_discard_info
#define elf_backend_ignore_discarded_relocs \
					_bfd_mips_elf_ignore_discarded_relocs
#define elf_backend_write_section	_bfd_mips_elf_write_section
#define elf_backend_elfsym_local_is_section \
					mips_elf32_elfsym_local_is_section
#define elf_backend_mips_irix_compat	elf32_mips_irix_compat
#define elf_backend_mips_rtype_to_howto	mips_elf32_rtype_to_howto
#define elf_backend_sort_relocs_p	_bfd_mips_elf_sort_relocs_p

#define bfd_elf32_bfd_is_local_label_name \
					mips_elf_is_local_label_name
#define bfd_elf32_bfd_is_target_special_symbol \
					_bfd_mips_elf_is_target_special_symbol
#define bfd_elf32_get_synthetic_symtab	_bfd_mips_elf_get_synthetic_symtab
#define bfd_elf32_find_nearest_line	_bfd_mips_elf_find_nearest_line
#define bfd_elf32_find_nearest_line_with_alt \
					_bfd_mips_elf_find_nearest_line_with_alt
#define bfd_elf32_find_inliner_info	_bfd_mips_elf_find_inliner_info
#define bfd_elf32_new_section_hook	_bfd_mips_elf_new_section_hook
#define bfd_elf32_set_section_contents	_bfd_mips_elf_set_section_contents
#define bfd_elf32_bfd_get_relocated_section_contents \
				_bfd_elf_mips_get_relocated_section_contents
#define bfd_elf32_bfd_link_hash_table_create \
					_bfd_mips_elf_link_hash_table_create
#define bfd_elf32_bfd_final_link	_bfd_mips_elf_final_link
#define bfd_elf32_bfd_merge_private_bfd_data \
					_bfd_mips_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags	_bfd_mips_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
					_bfd_mips_elf_print_private_bfd_data
#define bfd_elf32_bfd_relax_section	_bfd_mips_elf_relax_section
#define bfd_elf32_mkobject		_bfd_mips_elf_mkobject
#define bfd_elf32_bfd_free_cached_info	_bfd_mips_elf_free_cached_info

/* Support for SGI-ish mips targets.  */
#define TARGET_LITTLE_SYM		mips_elf32_le_vec
#define TARGET_LITTLE_NAME		"elf32-littlemips"
#define TARGET_BIG_SYM			mips_elf32_be_vec
#define TARGET_BIG_NAME			"elf32-bigmips"

/* The SVR4 MIPS ABI says that this should be 0x10000, but Irix 5 uses
   a value of 0x1000, and we are compatible.  */
#define ELF_MAXPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#include "elf32-target.h"

/* Support for traditional mips targets.  */
#undef TARGET_LITTLE_SYM
#undef TARGET_LITTLE_NAME
#undef TARGET_BIG_SYM
#undef TARGET_BIG_NAME

#undef ELF_MAXPAGESIZE
#undef ELF_COMMONPAGESIZE

#define TARGET_LITTLE_SYM		mips_elf32_trad_le_vec
#define TARGET_LITTLE_NAME		"elf32-tradlittlemips"
#define TARGET_BIG_SYM			mips_elf32_trad_be_vec
#define TARGET_BIG_NAME			"elf32-tradbigmips"

/* The MIPS ABI says at Page 5-1:
   Virtual addresses and file offsets for MIPS segments are congruent
   modulo 64 KByte (0x10000) or larger powers of 2.  Because 64 KBytes
   is the maximum page size, the files are suitable for paging
   regardless of physical page size.  */
#define ELF_MAXPAGESIZE			0x10000
#define ELF_COMMONPAGESIZE		0x1000
#define elf32_bed			elf32_tradbed

#undef elf_backend_write_core_note
#define elf_backend_write_core_note	elf32_mips_write_core_note

/* Include the target file again for this target.  */
#include "elf32-target.h"

/* FreeBSD support.  */

#undef TARGET_LITTLE_SYM
#undef TARGET_LITTLE_NAME
#undef TARGET_BIG_SYM
#undef TARGET_BIG_NAME

#define	TARGET_LITTLE_SYM		mips_elf32_tradfbsd_le_vec
#define	TARGET_LITTLE_NAME		"elf32-tradlittlemips-freebsd"
#define	TARGET_BIG_SYM			mips_elf32_tradfbsd_be_vec
#define	TARGET_BIG_NAME			"elf32-tradbigmips-freebsd"

#undef	ELF_OSABI
#define	ELF_OSABI			ELFOSABI_FREEBSD

#undef	elf32_bed
#define elf32_bed				elf32_fbsd_tradbed

#undef elf_backend_write_core_note

#include "elf32-target.h"
/* Implement elf_backend_final_write_processing for VxWorks.  */

static bool
mips_vxworks_final_write_processing (bfd *abfd)
{
  _bfd_mips_final_write_processing (abfd);
  return elf_vxworks_final_write_processing (abfd);
}

#undef TARGET_LITTLE_SYM
#undef TARGET_LITTLE_NAME
#undef TARGET_BIG_SYM
#undef TARGET_BIG_NAME

#undef ELF_MAXPAGESIZE
#undef ELF_COMMONPAGESIZE

#define TARGET_LITTLE_SYM		mips_elf32_vxworks_le_vec
#define TARGET_LITTLE_NAME		"elf32-littlemips-vxworks"
#define TARGET_BIG_SYM			mips_elf32_vxworks_be_vec
#define TARGET_BIG_NAME			"elf32-bigmips-vxworks"
#undef	ELF_OSABI

#undef elf32_bed
#define elf32_bed			elf32_mips_vxworks_bed

#define ELF_MAXPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#undef ELF_TARGET_OS
#define ELF_TARGET_OS			is_vxworks

#undef elf_backend_want_got_plt
#define elf_backend_want_got_plt		1
#undef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym		1
#undef elf_backend_may_use_rel_p
#define elf_backend_may_use_rel_p		0
#undef elf_backend_may_use_rela_p
#define elf_backend_may_use_rela_p		1
#undef elf_backend_default_use_rela_p
#define elf_backend_default_use_rela_p		1
#undef elf_backend_got_header_size
#define elf_backend_got_header_size		(4 * 3)
#undef elf_backend_dtrel_excludes_plt
#define elf_backend_dtrel_excludes_plt		1

#undef elf_backend_finish_dynamic_symbol
#define elf_backend_finish_dynamic_symbol \
  _bfd_mips_vxworks_finish_dynamic_symbol
#undef bfd_elf32_bfd_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_create \
  _bfd_mips_vxworks_link_hash_table_create
#undef elf_backend_add_symbol_hook
#define elf_backend_add_symbol_hook \
  elf_vxworks_add_symbol_hook
#undef elf_backend_link_output_symbol_hook
#define elf_backend_link_output_symbol_hook \
  elf_vxworks_link_output_symbol_hook
#undef elf_backend_emit_relocs
#define elf_backend_emit_relocs \
  elf_vxworks_emit_relocs
#undef elf_backend_final_write_processing
#define elf_backend_final_write_processing \
  mips_vxworks_final_write_processing

#undef elf_backend_additional_program_headers
#undef elf_backend_modify_segment_map
#undef elf_backend_symbol_processing
/* NOTE: elf_backend_rela_normal is not defined for MIPS.  */

#undef bfd_elf32_get_synthetic_symtab

#include "elf32-target.h"
