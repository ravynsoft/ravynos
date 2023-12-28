/* Xilinx MicroBlaze-specific support for 32-bit ELF

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */


#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/microblaze.h"
#include <assert.h>

#define	USE_RELA	/* Only USE_REL is actually significant, but this is
			   here are a reminder...  */
#define INST_WORD_SIZE 4

static int ro_small_data_pointer = 0;
static int rw_small_data_pointer = 0;

static reloc_howto_type * microblaze_elf_howto_table [(int) R_MICROBLAZE_max];

static reloc_howto_type microblaze_elf_howto_raw[] =
{
   /* This reloc does nothing.  */
   HOWTO (R_MICROBLAZE_NONE,	/* Type.  */
	  0,			/* Rightshift.  */
	  0,			/* Size.  */
	  0,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont,  /* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_NONE",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0,			/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A standard 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A standard PCREL 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_PCREL,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_PCREL",	/* Name.  */
	  true,			/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit PCREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_64_PCREL,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_64_PCREL",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* The low half of a PCREL 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_PCREL_LO,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_signed, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_32_PCREL_LO",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit relocation.  Table entry not really used.  */
   HOWTO (R_MICROBLAZE_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* The low half of a 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_LO,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_signed, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_LO", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Read-only small data section relocation.  */
   HOWTO (R_MICROBLAZE_SRO32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_SRO32", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Read-write small data area relocation.  */
   HOWTO (R_MICROBLAZE_SRW32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_SRW32", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* This reloc does nothing.	Used for relaxation.  */
   HOWTO (R_MICROBLAZE_64_NONE,	/* Type.  */
	  0,			/* Rightshift.  */
	  0,			/* Size.  */
	  0,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_64_NONE",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0,			/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Symbol Op Symbol relocation.  */
   HOWTO (R_MICROBLAZE_32_SYM_OP_SYM,		/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_SYM_OP_SYM",		/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* GNU extension to record C++ vtable hierarchy.  */
   HOWTO (R_MICROBLAZE_GNU_VTINHERIT, /* Type.  */
	  0,			 /* Rightshift.  */
	  4,			 /* Size.  */
	  0,			 /* Bitsize.  */
	  false,		 /* PC_relative.  */
	  0,			 /* Bitpos.  */
	  complain_overflow_dont,/* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_GNU_VTINHERIT", /* Name.  */
	  false,		 /* Partial Inplace.  */
	  0,			 /* Source Mask.  */
	  0,			 /* Dest Mask.  */
	  false),		 /* PC relative offset?  */

   /* GNU extension to record C++ vtable member usage.  */
   HOWTO (R_MICROBLAZE_GNU_VTENTRY,   /* Type.  */
	  0,			 /* Rightshift.  */
	  4,			 /* Size.  */
	  0,			 /* Bitsize.  */
	  false,		 /* PC_relative.  */
	  0,			 /* Bitpos.  */
	  complain_overflow_dont,/* Complain on overflow.  */
	  _bfd_elf_rel_vtable_reloc_fn,	 /* Special Function.  */
	  "R_MICROBLAZE_GNU_VTENTRY", /* Name.  */
	  false,		 /* Partial Inplace.  */
	  0,			 /* Source Mask.  */
	  0,			 /* Dest Mask.  */
	  false),		 /* PC relative offset?  */

   /* A 64 bit GOTPC relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTPC_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_GOTPC_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

     /* A 64 bit TEXTPCREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_TEXTPCREL_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_TEXTPCREL_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit GOT relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOT_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GOT_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

    /* A 64 bit TEXTREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_TEXTREL_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_TEXTREL_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A 64 bit PLT relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_PLT_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_PLT_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_REL,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_REL",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_JUMP_SLOT,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_JUMP_SLOT",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GLOB_DAT,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GLOB_DAT",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit GOT relative relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTOFF_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GOTOFF_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A 32 bit GOT relative relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTOFF_32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_GOTOFF_32",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* COPY relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_COPY,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_COPY",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Marker relocs for TLS.  */
   HOWTO (R_MICROBLAZE_TLS,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MICROBLAZE_TLS",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,			/* dst_mask */
	 false),		/* pcrel_offset */

   HOWTO (R_MICROBLAZE_TLSGD,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSGD",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,			/* dst_mask */
	 false),		/* pcrel_offset */

   HOWTO (R_MICROBLAZE_TLSLD,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSLD",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes the load module index of the load module that contains the
      definition of its TLS sym.  */
   HOWTO (R_MICROBLAZE_TLSDTPMOD32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPMOD32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a dtv-relative displacement, the difference between the value
      of sym+add and the base address of the thread-local storage block that
      contains the definition of sym, minus 0x8000.  Used for initializing GOT */
   HOWTO (R_MICROBLAZE_TLSDTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a dtv-relative displacement, the difference between the value
      of sym+add and the base address of the thread-local storage block that
      contains the definition of sym, minus 0x8000.  */
   HOWTO (R_MICROBLAZE_TLSDTPREL64,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPREL64",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a tp-relative displacement, the difference between the value of
      sym+add and the value of the thread pointer (r13).  */
   HOWTO (R_MICROBLAZE_TLSGOTTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSGOTTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a tp-relative displacement, the difference between the value of
      sym+add and the value of the thread pointer (r13).  */
   HOWTO (R_MICROBLAZE_TLSTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

};

#ifndef NUM_ELEM
#define NUM_ELEM(a) (sizeof (a) / sizeof (a)[0])
#endif

/* Initialize the microblaze_elf_howto_table, so that linear accesses can be done.  */

static void
microblaze_elf_howto_init (void)
{
  unsigned int i;

  for (i = NUM_ELEM (microblaze_elf_howto_raw); i--;)
    {
      unsigned int type;

      type = microblaze_elf_howto_raw[i].type;

      BFD_ASSERT (type < NUM_ELEM (microblaze_elf_howto_table));

      microblaze_elf_howto_table [type] = & microblaze_elf_howto_raw [i];
    }
}

static reloc_howto_type *
microblaze_elf_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				  bfd_reloc_code_real_type code)
{
  enum elf_microblaze_reloc_type microblaze_reloc = R_MICROBLAZE_NONE;

  switch (code)
    {
    case BFD_RELOC_NONE:
      microblaze_reloc = R_MICROBLAZE_NONE;
      break;
    case BFD_RELOC_MICROBLAZE_64_NONE:
      microblaze_reloc = R_MICROBLAZE_64_NONE;
      break;
    case BFD_RELOC_32:
      microblaze_reloc = R_MICROBLAZE_32;
      break;
      /* RVA is treated the same as 32 */
    case BFD_RELOC_RVA:
      microblaze_reloc = R_MICROBLAZE_32;
      break;
    case BFD_RELOC_32_PCREL:
      microblaze_reloc = R_MICROBLAZE_32_PCREL;
      break;
    case BFD_RELOC_64_PCREL:
      microblaze_reloc = R_MICROBLAZE_64_PCREL;
      break;
    case BFD_RELOC_MICROBLAZE_32_LO_PCREL:
      microblaze_reloc = R_MICROBLAZE_32_PCREL_LO;
      break;
    case BFD_RELOC_64:
      microblaze_reloc = R_MICROBLAZE_64;
      break;
    case BFD_RELOC_MICROBLAZE_32_LO:
      microblaze_reloc = R_MICROBLAZE_32_LO;
      break;
    case BFD_RELOC_MICROBLAZE_32_ROSDA:
      microblaze_reloc = R_MICROBLAZE_SRO32;
      break;
    case BFD_RELOC_MICROBLAZE_32_RWSDA:
      microblaze_reloc = R_MICROBLAZE_SRW32;
      break;
    case BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM:
      microblaze_reloc = R_MICROBLAZE_32_SYM_OP_SYM;
      break;
    case BFD_RELOC_VTABLE_INHERIT:
      microblaze_reloc = R_MICROBLAZE_GNU_VTINHERIT;
      break;
    case BFD_RELOC_VTABLE_ENTRY:
      microblaze_reloc = R_MICROBLAZE_GNU_VTENTRY;
      break;
    case BFD_RELOC_MICROBLAZE_64_GOTPC:
      microblaze_reloc = R_MICROBLAZE_GOTPC_64;
      break;
    case BFD_RELOC_MICROBLAZE_64_GOT:
      microblaze_reloc = R_MICROBLAZE_GOT_64;
      break;
    case BFD_RELOC_MICROBLAZE_64_TEXTPCREL:
      microblaze_reloc = R_MICROBLAZE_TEXTPCREL_64;
      break;
    case BFD_RELOC_MICROBLAZE_64_TEXTREL:
      microblaze_reloc = R_MICROBLAZE_TEXTREL_64;
      break;
    case BFD_RELOC_MICROBLAZE_64_PLT:
      microblaze_reloc = R_MICROBLAZE_PLT_64;
      break;
    case BFD_RELOC_MICROBLAZE_64_GOTOFF:
      microblaze_reloc = R_MICROBLAZE_GOTOFF_64;
      break;
    case BFD_RELOC_MICROBLAZE_32_GOTOFF:
      microblaze_reloc = R_MICROBLAZE_GOTOFF_32;
      break;
    case BFD_RELOC_MICROBLAZE_64_TLSGD:
      microblaze_reloc = R_MICROBLAZE_TLSGD;
      break;
    case BFD_RELOC_MICROBLAZE_64_TLSLD:
      microblaze_reloc = R_MICROBLAZE_TLSLD;
      break;
    case BFD_RELOC_MICROBLAZE_32_TLSDTPREL:
      microblaze_reloc = R_MICROBLAZE_TLSDTPREL32;
      break;
    case BFD_RELOC_MICROBLAZE_64_TLSDTPREL:
      microblaze_reloc = R_MICROBLAZE_TLSDTPREL64;
      break;
    case BFD_RELOC_MICROBLAZE_32_TLSDTPMOD:
      microblaze_reloc = R_MICROBLAZE_TLSDTPMOD32;
      break;
    case BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL:
      microblaze_reloc = R_MICROBLAZE_TLSGOTTPREL32;
      break;
    case BFD_RELOC_MICROBLAZE_64_TLSTPREL:
      microblaze_reloc = R_MICROBLAZE_TLSTPREL32;
      break;
    case BFD_RELOC_MICROBLAZE_COPY:
      microblaze_reloc = R_MICROBLAZE_COPY;
      break;
    default:
      return (reloc_howto_type *) NULL;
    }

  if (!microblaze_elf_howto_table [R_MICROBLAZE_32])
    /* Initialize howto table if needed.  */
    microblaze_elf_howto_init ();

  return microblaze_elf_howto_table [(int) microblaze_reloc];
};

static reloc_howto_type *
microblaze_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				  const char *r_name)
{
  unsigned int i;

  for (i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); i++)
    if (microblaze_elf_howto_raw[i].name != NULL
	&& strcasecmp (microblaze_elf_howto_raw[i].name, r_name) == 0)
      return &microblaze_elf_howto_raw[i];

  return NULL;
}

/* Set the howto pointer for a RCE ELF reloc.  */

static bool
microblaze_elf_info_to_howto (bfd * abfd,
			      arelent * cache_ptr,
			      Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  if (!microblaze_elf_howto_table [R_MICROBLAZE_32])
    /* Initialize howto table if needed.  */
    microblaze_elf_howto_init ();

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= R_MICROBLAZE_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = microblaze_elf_howto_table [r_type];
  return true;
}

/* Relax table contains information about instructions which can
   be removed by relaxation -- replacing a long address with a
   short address.  */
struct relax_table
{
  /* Address where bytes may be deleted.  */
  bfd_vma addr;

  /* Number of bytes to be deleted.  */
  size_t size;
};

struct _microblaze_elf_section_data
{
  struct bfd_elf_section_data elf;
  /* Count of used relaxation table entries.  */
  size_t relax_count;
  /* Relaxation table.  */
  struct relax_table *relax;
};

#define microblaze_elf_section_data(sec) \
  ((struct _microblaze_elf_section_data *) elf_section_data (sec))

static bool
microblaze_elf_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      struct _microblaze_elf_section_data *sdata;
      size_t amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Microblaze ELF local labels start with 'L.' or '$L', not '.L'.  */

static bool
microblaze_elf_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == 'L' && name[1] == '.')
    return true;

  if (name[0] == '$' && name[1] == 'L')
    return true;

  /* With gcc, the labels go back to starting with '.', so we accept
     the generic ELF local label syntax as well.  */
  return _bfd_elf_is_local_label_name (abfd, name);
}

/* ELF linker hash entry.  */

struct elf32_mb_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* TLS Reference Types for the symbol; Updated by check_relocs */
#define TLS_GD     1  /* GD reloc. */
#define TLS_LD     2  /* LD reloc. */
#define TLS_TPREL  4  /* TPREL reloc, => IE. */
#define TLS_DTPREL 8  /* DTPREL reloc, => LD. */
#define TLS_TLS    16 /* Any TLS reloc.  */
  unsigned char tls_mask;

};

#define IS_TLS_GD(x)     (x == (TLS_TLS | TLS_GD))
#define IS_TLS_LD(x)     (x == (TLS_TLS | TLS_LD))
#define IS_TLS_DTPREL(x) (x == (TLS_TLS | TLS_DTPREL))
#define IS_TLS_NONE(x)   (x == 0)

#define elf32_mb_hash_entry(ent) ((struct elf32_mb_link_hash_entry *)(ent))

/* ELF linker hash table.  */

struct elf32_mb_link_hash_table
{
  struct elf_link_hash_table elf;

  /* TLS Local Dynamic GOT Entry */
  union {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tlsld_got;
};

/* Nonzero if this section has TLS related relocations.  */
#define has_tls_reloc sec_flg0

/* Get the ELF linker hash table from a link_info structure.  */

#define elf32_mb_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == MICROBLAZE_ELF_DATA)	\
   ? (struct elf32_mb_link_hash_table *) (p)->hash : NULL)

/* Create an entry in a microblaze ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_mb_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_mb_link_hash_entry *eh;

      eh = (struct elf32_mb_link_hash_entry *) entry;
      eh->tls_mask = 0;
    }

  return entry;
}

/* Create a mb ELF linker hash table.  */

static struct bfd_link_hash_table *
microblaze_elf_link_hash_table_create (bfd *abfd)
{
  struct elf32_mb_link_hash_table *ret;
  size_t amt = sizeof (struct elf32_mb_link_hash_table);

  ret = (struct elf32_mb_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
				      sizeof (struct elf32_mb_link_hash_entry),
				      MICROBLAZE_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Set the values of the small data pointers.  */

static void
microblaze_elf_final_sdp (struct bfd_link_info *info)
{
  struct bfd_link_hash_entry *h;

  h = bfd_link_hash_lookup (info->hash, RO_SDA_ANCHOR_NAME, false, false, true);
  if (h != (struct bfd_link_hash_entry *) NULL
      && h->type == bfd_link_hash_defined)
    ro_small_data_pointer = (h->u.def.value
			     + h->u.def.section->output_section->vma
			     + h->u.def.section->output_offset);

  h = bfd_link_hash_lookup (info->hash, RW_SDA_ANCHOR_NAME, false, false, true);
  if (h != (struct bfd_link_hash_entry *) NULL
      && h->type == bfd_link_hash_defined)
    rw_small_data_pointer = (h->u.def.value
			     + h->u.def.section->output_section->vma
			     + h->u.def.section->output_offset);
}

static bfd_vma
dtprel_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* The size of the thread control block.  */
#define TCB_SIZE	8

/* Output a simple dynamic relocation into SRELOC.  */

static void
microblaze_elf_output_dynamic_relocation (bfd *output_bfd,
					  asection *sreloc,
					  unsigned long reloc_index,
					  unsigned long indx,
					  int r_type,
					  bfd_vma offset,
					  bfd_vma addend)
{

  Elf_Internal_Rela rel;

  rel.r_info = ELF32_R_INFO (indx, r_type);
  rel.r_offset = offset;
  rel.r_addend = addend;

  bfd_elf32_swap_reloca_out (output_bfd, &rel,
	      (sreloc->contents + reloc_index * sizeof (Elf32_External_Rela)));
}

/* This code is taken from elf32-m32r.c
   There is some attempt to make this function usable for many architectures,
   both USE_REL and USE_RELA ['twould be nice if such a critter existed],
   if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

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
microblaze_elf_relocate_section (bfd *output_bfd,
				 struct bfd_link_info *info,
				 bfd *input_bfd,
				 asection *input_section,
				 bfd_byte *contents,
				 Elf_Internal_Rela *relocs,
				 Elf_Internal_Sym *local_syms,
				 asection **local_sections)
{
  struct elf32_mb_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  Elf_Internal_Rela *rel, *relend;
  int endian = (bfd_little_endian (output_bfd)) ? 0 : 2;
  /* Assume success.  */
  bool ret = true;
  asection *sreloc;
  bfd_vma *local_got_offsets;
  unsigned int tls_type;

  if (!microblaze_elf_howto_table[R_MICROBLAZE_max-1])
    microblaze_elf_howto_init ();

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  local_got_offsets = elf_local_got_offsets (input_bfd);

  sreloc = elf_section_data (input_section)->sreloc;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      bfd_vma addend = rel->r_addend;
      bfd_vma offset = rel->r_offset;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      const char *sym_name;
      bfd_reloc_status_type r = bfd_reloc_ok;
      const char *errmsg = NULL;
      bool unresolved_reloc = false;

      h = NULL;
      r_type = ELF32_R_TYPE (rel->r_info);
      tls_type = 0;

      if (r_type < 0 || r_type >= (int) R_MICROBLAZE_max)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, (int) r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret = false;
	  continue;
	}

      howto = microblaze_elf_howto_table[r_type];
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (bfd_link_relocatable (info))
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  sec = NULL;
	  if (r_symndx >= symtab_hdr->sh_info)
	    /* External symbol.  */
	    continue;

	  /* Local symbol.  */
	  sym = local_syms + r_symndx;
	  sym_name = "<local symbol>";
	  /* STT_SECTION: symbol is associated with a section.  */
	  if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
	    /* Symbol isn't associated with a section.  Nothing to do.  */
	    continue;

	  sec = local_sections[r_symndx];
	  addend += sec->output_offset + sym->st_value;
#ifndef USE_REL
	  /* This can't be done for USE_REL because it doesn't mean anything
	     and elf_link_input_bfd asserts this stays zero.  */
	  /* rel->r_addend = addend; */
#endif

#ifndef USE_REL
	  /* Addends are stored with relocs.  We're done.  */
	  continue;
#else /* USE_REL */
	  /* If partial_inplace, we need to store any additional addend
	     back in the section.  */
	  if (!howto->partial_inplace)
	    continue;
	  /* ??? Here is a nice place to call a special_function like handler.  */
	  r = _bfd_relocate_contents (howto, input_bfd, addend,
				      contents + offset);
#endif /* USE_REL */
	}
      else
	{
	  bfd_vma relocation;
	  bool resolved_to_zero;

	  /* This is a final link.  */
	  sym = NULL;
	  sec = NULL;
	  unresolved_reloc = false;

	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      /* Local symbol.  */
	      sym = local_syms + r_symndx;
	      sec = local_sections[r_symndx];
	      if (sec == 0)
		continue;
	      sym_name = "<local symbol>";
	      relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	      /* r_addend may have changed if the reference section was
		 a merge section.  */
	      addend = rel->r_addend;
	    }
	  else
	    {
	      /* External symbol.  */
	      bool warned ATTRIBUTE_UNUSED;
	      bool ignored ATTRIBUTE_UNUSED;

	      RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				       r_symndx, symtab_hdr, sym_hashes,
				       h, sec, relocation,
				       unresolved_reloc, warned, ignored);
	      sym_name = h->root.root.string;
	    }

	  /* Sanity check the address.  */
	  if (offset > bfd_get_section_limit (input_bfd, input_section))
	    {
	      r = bfd_reloc_outofrange;
	      goto check_reloc;
	    }

	  resolved_to_zero = (h != NULL
			      && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

	  switch ((int) r_type)
	    {
	    case (int) R_MICROBLAZE_SRO32 :
	      {
		const char *name;

		/* Only relocate if the symbol is defined.  */
		if (sec)
		  {
		    name = bfd_section_name (sec);

		    if (strcmp (name, ".sdata2") == 0
			|| strcmp (name, ".sbss2") == 0)
		      {
			if (ro_small_data_pointer == 0)
			  microblaze_elf_final_sdp (info);
			if (ro_small_data_pointer == 0)
			  {
			    ret = false;
			    r = bfd_reloc_undefined;
			    goto check_reloc;
			  }

			/* At this point `relocation' contains the object's
			   address.  */
			relocation -= ro_small_data_pointer;
			/* Now it contains the offset from _SDA2_BASE_.  */
			r = _bfd_final_link_relocate (howto, input_bfd,
						      input_section,
						      contents, offset,
						      relocation, addend);
		      }
		    else
		      {
			_bfd_error_handler
			  /* xgettext:c-format */
			  (_("%pB: the target (%s) of an %s relocation"
			     " is in the wrong section (%pA)"),
			   input_bfd,
			   sym_name,
			   microblaze_elf_howto_table[(int) r_type]->name,
			   sec);
			/*bfd_set_error (bfd_error_bad_value); ??? why? */
			ret = false;
			continue;
		      }
		  }
	      }
	      break;

	    case (int) R_MICROBLAZE_SRW32 :
	      {
		const char *name;

		/* Only relocate if the symbol is defined.  */
		if (sec)
		  {
		    name = bfd_section_name (sec);

		    if (strcmp (name, ".sdata") == 0
			|| strcmp (name, ".sbss") == 0)
		      {
			if (rw_small_data_pointer == 0)
			  microblaze_elf_final_sdp (info);
			if (rw_small_data_pointer == 0)
			  {
			    ret = false;
			    r = bfd_reloc_undefined;
			    goto check_reloc;
			  }

			/* At this point `relocation' contains the object's
			   address.  */
			relocation -= rw_small_data_pointer;
			/* Now it contains the offset from _SDA_BASE_.  */
			r = _bfd_final_link_relocate (howto, input_bfd,
						      input_section,
						      contents, offset,
						      relocation, addend);
		      }
		    else
		      {
			_bfd_error_handler
			  /* xgettext:c-format */
			  (_("%pB: the target (%s) of an %s relocation"
			     " is in the wrong section (%pA)"),
			   input_bfd,
			   sym_name,
			   microblaze_elf_howto_table[(int) r_type]->name,
			   sec);
			/*bfd_set_error (bfd_error_bad_value); ??? why? */
			ret = false;
			continue;
		      }
		  }
	      }
	      break;

	    case (int) R_MICROBLAZE_32_SYM_OP_SYM:
	      break; /* Do nothing.  */

	    case (int) R_MICROBLAZE_GOTPC_64:
	      relocation = (htab->elf.sgotplt->output_section->vma
			    + htab->elf.sgotplt->output_offset);
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + offset + INST_WORD_SIZE);
	      relocation += addend;
	      bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
			  contents + offset + endian);
	      bfd_put_16 (input_bfd, relocation & 0xffff,
			  contents + offset + endian + INST_WORD_SIZE);
	      break;

	    case (int) R_MICROBLAZE_TEXTPCREL_64:
	      relocation = input_section->output_section->vma;
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + offset + INST_WORD_SIZE);
	      relocation += addend;
	      bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
			  contents + offset + endian);
	      bfd_put_16 (input_bfd, relocation & 0xffff,
			  contents + offset + endian + INST_WORD_SIZE);
	      break;

	    case (int) R_MICROBLAZE_PLT_64:
	      {
		bfd_vma immediate;
		if (htab->elf.splt != NULL && h != NULL
		    && h->plt.offset != (bfd_vma) -1)
		  {
		    relocation = (htab->elf.splt->output_section->vma
				  + htab->elf.splt->output_offset
				  + h->plt.offset);
		    unresolved_reloc = false;
		    immediate = relocation - (input_section->output_section->vma
					      + input_section->output_offset
					      + offset + INST_WORD_SIZE);
		    bfd_put_16 (input_bfd, (immediate >> 16) & 0xffff,
				contents + offset + endian);
		    bfd_put_16 (input_bfd, immediate & 0xffff,
				contents + offset + endian + INST_WORD_SIZE);
		  }
		else
		  {
		    relocation -= (input_section->output_section->vma
				   + input_section->output_offset
				   + offset + INST_WORD_SIZE);
		    immediate = relocation;
		    bfd_put_16 (input_bfd, (immediate >> 16) & 0xffff,
				contents + offset + endian);
		    bfd_put_16 (input_bfd, immediate & 0xffff,
				contents + offset + endian + INST_WORD_SIZE);
		  }
		break;
	      }

	    case (int) R_MICROBLAZE_TLSGD:
	      tls_type = (TLS_TLS | TLS_GD);
	      goto dogot;
	    case (int) R_MICROBLAZE_TLSLD:
	      tls_type = (TLS_TLS | TLS_LD);
	      /* Fall through.  */
	    dogot:
	    case (int) R_MICROBLAZE_GOT_64:
	      {
		bfd_vma *offp;
		bfd_vma off, off2;
		unsigned long indx;
		bfd_vma static_value;

		bool need_relocs = false;
		if (htab->elf.sgot == NULL)
		  abort ();

		indx = 0;
		offp = NULL;

		/* 1. Identify GOT Offset;
		   2. Compute Static Values
		   3. Process Module Id, Process Offset
		   4. Fixup Relocation with GOT offset value. */

		/* 1. Determine GOT Offset to use : TLS_LD, global, local */
		if (IS_TLS_LD (tls_type))
		  offp = &htab->tlsld_got.offset;
		else if (h != NULL)
		  {
		    if (htab->elf.sgotplt != NULL
			&& h->got.offset != (bfd_vma) -1)
		      offp = &h->got.offset;
		    else
		      abort ();
		  }
		else
		  {
		    if (local_got_offsets == NULL)
		      abort ();
		    offp = &local_got_offsets[r_symndx];
		  }

		if (!offp)
		  abort ();

		off = (*offp) & ~1;
		off2 = off;

		if (IS_TLS_LD(tls_type) || IS_TLS_GD(tls_type))
		  off2 = off + 4;

		/* Symbol index to use for relocs */
		if (h != NULL)
		  {
		    bool dyn =
			elf_hash_table (info)->dynamic_sections_created;

		    if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							 bfd_link_pic (info),
							 h)
			&& (!bfd_link_pic (info)
			    || !SYMBOL_REFERENCES_LOCAL (info, h)))
		      indx = h->dynindx;
		  }

		/* Need to generate relocs ? */
		if ((bfd_link_pic (info) || indx != 0)
		    && (h == NULL
		    || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			&& !resolved_to_zero)
		    || h->root.type != bfd_link_hash_undefweak))
		  need_relocs = true;

		/* 2. Compute/Emit Static value of r-expression */
		static_value = relocation + addend;

		/* 3. Process module-id and offset */
		if (! ((*offp) & 1) )
		  {
		    bfd_vma got_offset;

		    got_offset = (htab->elf.sgot->output_section->vma
				  + htab->elf.sgot->output_offset
				  + off);

		    /* Process module-id */
		    if (IS_TLS_LD(tls_type))
		      {
			if (! bfd_link_pic (info))
			  bfd_put_32 (output_bfd, 1,
				      htab->elf.sgot->contents + off);
			else
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd,
			     htab->elf.srelgot,
			     htab->elf.srelgot->reloc_count++,
			     /* symindex= */ 0, R_MICROBLAZE_TLSDTPMOD32,
			     got_offset, 0);
		      }
		    else if (IS_TLS_GD(tls_type))
		      {
			if (! need_relocs)
			  bfd_put_32 (output_bfd, 1,
				      htab->elf.sgot->contents + off);
			else
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd,
			     htab->elf.srelgot,
			     htab->elf.srelgot->reloc_count++,
			     /* symindex= */ indx, R_MICROBLAZE_TLSDTPMOD32,
			     got_offset, indx ? 0 : static_value);
		      }

		    /* Process Offset */
		    if (htab->elf.srelgot == NULL)
		      abort ();

		    got_offset = (htab->elf.sgot->output_section->vma
				  + htab->elf.sgot->output_offset
				  + off2);
		    if (IS_TLS_LD(tls_type))
		      {
			/* For LD, offset should be 0 */
			*offp |= 1;
			bfd_put_32 (output_bfd, 0,
				    htab->elf.sgot->contents + off2);
		      }
		    else if (IS_TLS_GD(tls_type))
		      {
			*offp |= 1;
			static_value -= dtprel_base(info);
			if (need_relocs)
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd,
			     htab->elf.srelgot,
			     htab->elf.srelgot->reloc_count++,
			     /* symindex= */ indx, R_MICROBLAZE_TLSDTPREL32,
			     got_offset, indx ? 0 : static_value);
			else
			  bfd_put_32 (output_bfd, static_value,
				      htab->elf.sgot->contents + off2);
		      }
		    else
		      {
			bfd_put_32 (output_bfd, static_value,
				    htab->elf.sgot->contents + off2);

			/* Relocs for dyn symbols generated by
			   finish_dynamic_symbols */
			if (bfd_link_pic (info) && h == NULL)
			  {
			    *offp |= 1;
			    microblaze_elf_output_dynamic_relocation
			      (output_bfd,
			       htab->elf.srelgot,
			       htab->elf.srelgot->reloc_count++,
			       /* symindex= */ indx, R_MICROBLAZE_REL,
			       got_offset, static_value);
			  }
		      }
		  }

		/* 4. Fixup Relocation with GOT offset value
		      Compute relative address of GOT entry for applying
		      the current relocation */
		relocation = htab->elf.sgot->output_section->vma
			     + htab->elf.sgot->output_offset
			     + off
			     - htab->elf.sgotplt->output_section->vma
			     - htab->elf.sgotplt->output_offset;

		/* Apply Current Relocation */
		bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
			    contents + offset + endian);
		bfd_put_16 (input_bfd, relocation & 0xffff,
			    contents + offset + endian + INST_WORD_SIZE);

		unresolved_reloc = false;
		break;
	      }

	    case (int) R_MICROBLAZE_GOTOFF_64:
	      {
		bfd_vma immediate;
		unsigned short lo, high;
		relocation += addend;
		relocation -= (htab->elf.sgotplt->output_section->vma
			       + htab->elf.sgotplt->output_offset);
		/* Write this value into correct location.  */
		immediate = relocation;
		lo = immediate & 0x0000ffff;
		high = (immediate >> 16) & 0x0000ffff;
		bfd_put_16 (input_bfd, high, contents + offset + endian);
		bfd_put_16 (input_bfd, lo,
			    contents + offset + INST_WORD_SIZE + endian);
		break;
	      }

	    case (int) R_MICROBLAZE_GOTOFF_32:
	      {
		relocation += addend;
		relocation -= (htab->elf.sgotplt->output_section->vma
			       + htab->elf.sgotplt->output_offset);
		/* Write this value into correct location.  */
		bfd_put_32 (input_bfd, relocation, contents + offset);
		break;
	      }

	    case (int) R_MICROBLAZE_TLSDTPREL64:
	      relocation += addend;
	      relocation -= dtprel_base(info);
	      bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
			  contents + offset + 2);
	      bfd_put_16 (input_bfd, relocation & 0xffff,
			  contents + offset + 2 + INST_WORD_SIZE);
	      break;
	    case (int) R_MICROBLAZE_TEXTREL_64:
	    case (int) R_MICROBLAZE_TEXTREL_32_LO:
	    case (int) R_MICROBLAZE_64_PCREL :
	    case (int) R_MICROBLAZE_64:
	    case (int) R_MICROBLAZE_32:
	      {
		/* r_symndx will be STN_UNDEF (zero) only for relocs against symbols
		   from removed linkonce sections, or sections discarded by
		   a linker script.  */
		if (r_symndx == STN_UNDEF || (input_section->flags & SEC_ALLOC) == 0)
		  {
		    relocation += addend;
		    if (r_type == R_MICROBLAZE_32)
		      bfd_put_32 (input_bfd, relocation, contents + offset);
		    else
		      {
			if (r_type == R_MICROBLAZE_64_PCREL)
			  relocation -= (input_section->output_section->vma
					 + input_section->output_offset
					 + offset + INST_WORD_SIZE);
			else if (r_type == R_MICROBLAZE_TEXTREL_64
				 || r_type == R_MICROBLAZE_TEXTREL_32_LO)
			  relocation -= input_section->output_section->vma;

			if (r_type == R_MICROBLAZE_TEXTREL_32_LO)
			  bfd_put_16 (input_bfd, relocation & 0xffff,
				      contents + offset + endian);

			else
			  {
			    bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
				    contents + offset + endian);
			    bfd_put_16 (input_bfd, relocation & 0xffff,
				    contents + offset + endian + INST_WORD_SIZE);
		      }
		    }
		    break;
		  }

		if ((bfd_link_pic (info)
		     && (h == NULL
			 || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			     && !resolved_to_zero)
			 || h->root.type != bfd_link_hash_undefweak)
		     && (!howto->pc_relative
			 || (h != NULL
			     && h->dynindx != -1
			     && (!info->symbolic
				 || !h->def_regular))))
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
		    bfd_byte *loc;
		    bool skip;

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
		      skip = true;
		    outrel.r_offset += (input_section->output_section->vma
					+ input_section->output_offset);

		    if (skip)
		      memset (&outrel, 0, sizeof outrel);
		    /* h->dynindx may be -1 if the symbol was marked to
		       become local.  */
		    else if (h != NULL
			     && ((! info->symbolic && h->dynindx != -1)
				 || !h->def_regular))
		      {
			BFD_ASSERT (h->dynindx != -1);
			outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
			outrel.r_addend = addend;
		      }
		    else
		      {
			if (r_type == R_MICROBLAZE_32)
			  {
			    outrel.r_info = ELF32_R_INFO (0, R_MICROBLAZE_REL);
			    outrel.r_addend = relocation + addend;
			  }
			else
			  {
			    BFD_FAIL ();
			    _bfd_error_handler
			      (_("%pB: probably compiled without -fPIC?"),
			       input_bfd);
			    bfd_set_error (bfd_error_bad_value);
			    return false;
			  }
		      }

		    loc = sreloc->contents;
		    loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
		    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    break;
		  }
		else
		  {
		    relocation += addend;
		    if (r_type == R_MICROBLAZE_32)
		      bfd_put_32 (input_bfd, relocation, contents + offset);
		    else
		      {
			if (r_type == R_MICROBLAZE_64_PCREL)
			  relocation -= (input_section->output_section->vma
					 + input_section->output_offset
					 + offset + INST_WORD_SIZE);
			else if (r_type == R_MICROBLAZE_TEXTREL_64
				 || r_type == R_MICROBLAZE_TEXTREL_32_LO)
			  relocation -= input_section->output_section->vma;

			if (r_type == R_MICROBLAZE_TEXTREL_32_LO)
			  {
			     bfd_put_16 (input_bfd, relocation & 0xffff,
					 contents + offset + endian);
			  }
			else
			  {
			    bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
				        contents + offset + endian);
			    bfd_put_16 (input_bfd, relocation & 0xffff,
					contents + offset + endian
					+ INST_WORD_SIZE);
			  }
		    }
		    break;
		  }
	      }

	    default :
	      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					    contents, offset,
					    relocation, addend);
	      break;
	    }
	}

    check_reloc:

      if (r != bfd_reloc_ok)
	{
	  /* FIXME: This should be generic enough to go in a utility.  */
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

	  if (errmsg != NULL)
	    goto common_error;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), name, howto->name,
		 (bfd_vma) 0, input_bfd, input_section, offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      errmsg = _("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      errmsg = _("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      errmsg = _("internal error: dangerous error");
	      goto common_error;

	    default:
	      errmsg = _("internal error: unknown error");
	      /* Fall through.  */
	    common_error:
	      (*info->callbacks->warning) (info, errmsg, name, input_bfd,
					   input_section, offset);
	      break;
	    }
	}
    }

  return ret;
}

/* Calculate fixup value for reference.  */

static size_t
calc_fixup (bfd_vma start, bfd_vma size, asection *sec)
{
  bfd_vma end = start + size;
  size_t i, fixup = 0;
  struct _microblaze_elf_section_data *sdata;

  if (sec == NULL || (sdata = microblaze_elf_section_data (sec)) == NULL)
    return 0;

  /* Look for addr in relax table, total fixup value.  */
  for (i = 0; i < sdata->relax_count; i++)
    {
      if (end <= sdata->relax[i].addr)
	break;
      if (end != start && start > sdata->relax[i].addr)
	continue;
      fixup += sdata->relax[i].size;
    }
  return fixup;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   a 32-bit instruction.  */
static void
microblaze_bfd_write_imm_value_32 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    unsigned long instr = bfd_get_32 (abfd, bfd_addr);
    instr &= ~0x0000ffff;
    instr |= (val & 0x0000ffff);
    bfd_put_32 (abfd, instr, bfd_addr);
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   two consecutive 32-bit instructions.  */
static void
microblaze_bfd_write_imm_value_64 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    unsigned long instr_hi;
    unsigned long instr_lo;

    instr_hi = bfd_get_32 (abfd, bfd_addr);
    instr_hi &= ~0x0000ffff;
    instr_hi |= ((val >> 16) & 0x0000ffff);
    bfd_put_32 (abfd, instr_hi, bfd_addr);

    instr_lo = bfd_get_32 (abfd, bfd_addr + INST_WORD_SIZE);
    instr_lo &= ~0x0000ffff;
    instr_lo |= (val & 0x0000ffff);
    bfd_put_32 (abfd, instr_lo, bfd_addr + INST_WORD_SIZE);
}

static bool
microblaze_elf_relax_section (bfd *abfd,
			      asection *sec,
			      struct bfd_link_info *link_info,
			      bool *again)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *free_relocs = NULL;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;
  bfd_byte *free_contents = NULL;
  int rel_count;
  unsigned int shndx;
  size_t i, sym_index;
  asection *o;
  struct elf_link_hash_entry *sym_hash;
  Elf_Internal_Sym *isymbuf, *isymend;
  Elf_Internal_Sym *isym;
  size_t symcount;
  size_t offset;
  bfd_vma src, dest;
  struct _microblaze_elf_section_data *sdata;

  /* We only do this once per section.  We may be able to delete some code
     by running multiple passes, but it is not worth it.  */
  *again = false;

  /* Only do this for a text section.  */
  if (bfd_link_relocatable (link_info)
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_CODE) == 0
      || sec->reloc_count == 0
      || (sdata = microblaze_elf_section_data (sec)) == NULL)
    return true;

  BFD_ASSERT ((sec->size > 0) || (sec->rawsize > 0));

  /* If this is the first time we have been called for this section,
     initialize the cooked size.  */
  if (sec->size == 0)
    sec->size = sec->rawsize;

  /* Get symbols for this section.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
  symcount =  symtab_hdr->sh_size / sizeof (Elf32_External_Sym);
  if (isymbuf == NULL)
    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr, symcount,
				    0, NULL, NULL, NULL);
  BFD_ASSERT (isymbuf != NULL);

  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL, link_info->keep_memory);
  if (internal_relocs == NULL)
    goto error_return;
  if (! link_info->keep_memory)
    free_relocs = internal_relocs;

  sdata->relax_count = 0;
  sdata->relax = (struct relax_table *) bfd_malloc ((sec->reloc_count + 1)
						    * sizeof (*sdata->relax));
  if (sdata->relax == NULL)
    goto error_return;

  irelend = internal_relocs + sec->reloc_count;
  rel_count = 0;
  for (irel = internal_relocs; irel < irelend; irel++, rel_count++)
    {
      bfd_vma symval;
      if ((ELF32_R_TYPE (irel->r_info) != (int) R_MICROBLAZE_64_PCREL)
	  && (ELF32_R_TYPE (irel->r_info) != (int) R_MICROBLAZE_64)
	  && (ELF32_R_TYPE (irel->r_info) != (int) R_MICROBLAZE_TEXTREL_64))
	continue; /* Can't delete this reloc.  */

      /* Get the section contents.  */
      if (contents == NULL)
	{
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  else
	    {
	      contents = (bfd_byte *) bfd_malloc (sec->size);
	      if (contents == NULL)
		goto error_return;
	      free_contents = contents;

	      if (!bfd_get_section_contents (abfd, sec, contents,
					     (file_ptr) 0, sec->size))
		goto error_return;
	      elf_section_data (sec)->this_hdr.contents = contents;
	    }
	}

      /* Get the value of the symbol referred to by the reloc.  */
      if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
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

	  symval = _bfd_elf_rela_local_sym (abfd, isym, &sym_sec, irel);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

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
	}

      /* If this is a PC-relative reloc, subtract the instr offset from
	 the symbol value.  */
      if (ELF32_R_TYPE (irel->r_info) == (int) R_MICROBLAZE_64_PCREL)
	{
	  symval = symval + irel->r_addend
	    - (irel->r_offset
	       + sec->output_section->vma
	       + sec->output_offset);
	}
      else if (ELF32_R_TYPE (irel->r_info) == (int) R_MICROBLAZE_TEXTREL_64)
	{
	  symval = symval + irel->r_addend - (sec->output_section->vma);
	}
      else
	symval += irel->r_addend;

      if ((symval & 0xffff8000) == 0
	  || (symval & 0xffff8000) == 0xffff8000)
	{
	  /* We can delete this instruction.  */
	  sdata->relax[sdata->relax_count].addr = irel->r_offset;
	  sdata->relax[sdata->relax_count].size = INST_WORD_SIZE;
	  sdata->relax_count++;

	  /* Rewrite relocation type.  */
	  switch ((enum elf_microblaze_reloc_type) ELF32_R_TYPE (irel->r_info))
	    {
	    case R_MICROBLAZE_64_PCREL:
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   (int) R_MICROBLAZE_32_PCREL_LO);
	      break;
	    case R_MICROBLAZE_64:
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   (int) R_MICROBLAZE_32_LO);
	      break;
	    case R_MICROBLAZE_TEXTREL_64:
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   (int) R_MICROBLAZE_TEXTREL_32_LO);
	      break;
	    default:
	      /* Cannot happen.  */
	      BFD_ASSERT (false);
	    }
	}
    } /* Loop through all relocations.  */

  /* Loop through the relocs again, and see if anything needs to change.  */
  if (sdata->relax_count > 0)
    {
      shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
      rel_count = 0;
      sdata->relax[sdata->relax_count].addr = sec->size;

      for (irel = internal_relocs; irel < irelend; irel++, rel_count++)
	{
	  bfd_vma nraddr;

	  /* Get the new reloc address.  */
	  nraddr = irel->r_offset - calc_fixup (irel->r_offset, 0, sec);
	  switch ((enum elf_microblaze_reloc_type) ELF32_R_TYPE (irel->r_info))
	    {
	    default:
	      break;
	    case R_MICROBLAZE_64_PCREL:
	      break;
	    case R_MICROBLAZE_TEXTREL_64:
	    case R_MICROBLAZE_TEXTREL_32_LO:
	    case R_MICROBLAZE_64:
	    case R_MICROBLAZE_32_LO:
	      /* If this reloc is against a symbol defined in this
		 section, we must check the addend to see it will put the value in
		 range to be adjusted, and hence must be changed.  */
	      if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
		{
		  isym = isymbuf + ELF32_R_SYM (irel->r_info);
		  /* Only handle relocs against .text.  */
		  if (isym->st_shndx == shndx
		      && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
		    irel->r_addend -= calc_fixup (irel->r_addend, 0, sec);
		}
	      break;
	    case R_MICROBLAZE_NONE:
	      {
		/* This was a PC-relative instruction that was
		   completely resolved.  */
		size_t sfix, efix;
		bfd_vma target_address;
		target_address = irel->r_addend + irel->r_offset;
		sfix = calc_fixup (irel->r_offset, 0, sec);
		efix = calc_fixup (target_address, 0, sec);
		irel->r_addend -= (efix - sfix);
		/* Should use HOWTO.  */
		microblaze_bfd_write_imm_value_32 (abfd, contents + irel->r_offset,
						   irel->r_addend);
	      }
	      break;
	    case R_MICROBLAZE_64_NONE:
	      {
		/* This was a PC-relative 64-bit instruction that was
		   completely resolved.  */
		size_t sfix, efix;
		bfd_vma target_address;
		target_address = irel->r_addend + irel->r_offset + INST_WORD_SIZE;
		sfix = calc_fixup (irel->r_offset + INST_WORD_SIZE, 0, sec);
		efix = calc_fixup (target_address, 0, sec);
		irel->r_addend -= (efix - sfix);
    microblaze_bfd_write_imm_value_32 (abfd, contents + irel->r_offset
				       + INST_WORD_SIZE, irel->r_addend);
	      }
	      break;
	    }
	  irel->r_offset = nraddr;
	} /* Change all relocs in this section.  */

      /* Look through all other sections.  */
      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  Elf_Internal_Rela *irelocs;
	  Elf_Internal_Rela *irelscan, *irelscanend;
	  bfd_byte *ocontents;

	  if (o == sec
	      || (o->flags & SEC_RELOC) == 0
	      || o->reloc_count == 0)
	    continue;

	  /* We always cache the relocs.  Perhaps, if info->keep_memory is
	     FALSE, we should free them, if we are permitted to.  */

	  irelocs = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL, true);
	  if (irelocs == NULL)
	    goto error_return;

	  ocontents = NULL;
	  irelscanend = irelocs + o->reloc_count;
	  for (irelscan = irelocs; irelscan < irelscanend; irelscan++)
	    {
	      if (ELF32_R_TYPE (irelscan->r_info) == (int) R_MICROBLAZE_32)
		{
		  isym = isymbuf + ELF32_R_SYM (irelscan->r_info);

		  /* Look at the reloc only if the value has been resolved.  */
		  if (isym->st_shndx == shndx
		      && (ELF32_ST_TYPE (isym->st_info) == STT_SECTION))
		    {
		      if (ocontents == NULL)
			{
			  if (elf_section_data (o)->this_hdr.contents != NULL)
			    ocontents = elf_section_data (o)->this_hdr.contents;
			  else
			    {
			      /* We always cache the section contents.
				 Perhaps, if info->keep_memory is FALSE, we
				 should free them, if we are permitted to.  */
			      if (o->rawsize == 0)
				o->rawsize = o->size;
			      ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
			      if (ocontents == NULL)
				goto error_return;
			      if (!bfd_get_section_contents (abfd, o, ocontents,
							     (file_ptr) 0,
							     o->rawsize))
				goto error_return;
			      elf_section_data (o)->this_hdr.contents = ocontents;
			    }

			}
		      irelscan->r_addend -= calc_fixup (irelscan->r_addend, 0, sec);
		    }
		  else if (ELF32_R_TYPE (irelscan->r_info) == (int) R_MICROBLAZE_32_SYM_OP_SYM)
		    {
		      isym = isymbuf + ELF32_R_SYM (irelscan->r_info);

		      /* Look at the reloc only if the value has been resolved.  */
		      if (ocontents == NULL)
			{
			  if (elf_section_data (o)->this_hdr.contents != NULL)
			    ocontents = elf_section_data (o)->this_hdr.contents;
			  else
			    {
			      /* We always cache the section contents.
				 Perhaps, if info->keep_memory is FALSE, we
				 should free them, if we are permitted to.  */

			      if (o->rawsize == 0)
				o->rawsize = o->size;
			      ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
			      if (ocontents == NULL)
				goto error_return;
			      if (!bfd_get_section_contents (abfd, o, ocontents,
							     (file_ptr) 0,
							     o->rawsize))
				goto error_return;
			      elf_section_data (o)->this_hdr.contents = ocontents;
			    }
			}
		      irelscan->r_addend -= calc_fixup (irel->r_addend
							+ isym->st_value,
							0,
							sec);
		    }
		}
	      else if ((ELF32_R_TYPE (irelscan->r_info) == (int) R_MICROBLAZE_32_PCREL_LO)
		       || (ELF32_R_TYPE (irelscan->r_info)
				   == (int) R_MICROBLAZE_32_LO)
		       || (ELF32_R_TYPE (irelscan->r_info)
				   == (int) R_MICROBLAZE_TEXTREL_32_LO))
		{
		  isym = isymbuf + ELF32_R_SYM (irelscan->r_info);

		  /* Look at the reloc only if the value has been resolved.  */
		  if (isym->st_shndx == shndx
		      && (ELF32_ST_TYPE (isym->st_info) == STT_SECTION))
		    {
		      bfd_vma immediate;
		      bfd_vma target_address;

		      if (ocontents == NULL)
			{
			  if (elf_section_data (o)->this_hdr.contents != NULL)
			    ocontents = elf_section_data (o)->this_hdr.contents;
			  else
			    {
			      /* We always cache the section contents.
				 Perhaps, if info->keep_memory is FALSE, we
				 should free them, if we are permitted to.  */
			      if (o->rawsize == 0)
				o->rawsize = o->size;
			      ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
			      if (ocontents == NULL)
				goto error_return;
			      if (!bfd_get_section_contents (abfd, o, ocontents,
							     (file_ptr) 0,
							     o->rawsize))
				goto error_return;
			      elf_section_data (o)->this_hdr.contents = ocontents;
			    }
			}

		      unsigned long instr = bfd_get_32 (abfd, ocontents + irelscan->r_offset);
		      immediate = instr & 0x0000ffff;
		      target_address = immediate;
		      offset = calc_fixup (target_address, 0, sec);
		      immediate -= offset;
		      irelscan->r_addend -= offset;
	  microblaze_bfd_write_imm_value_32 (abfd, ocontents + irelscan->r_offset,
					     irelscan->r_addend);
		    }
		}

	      if (ELF32_R_TYPE (irelscan->r_info) == (int) R_MICROBLAZE_64
		  || (ELF32_R_TYPE (irelscan->r_info)
			      == (int) R_MICROBLAZE_TEXTREL_64))
		{
		  isym = isymbuf + ELF32_R_SYM (irelscan->r_info);

		  /* Look at the reloc only if the value has been resolved.  */
		  if (isym->st_shndx == shndx
		      && (ELF32_ST_TYPE (isym->st_info) == STT_SECTION))
		    {
		      if (ocontents == NULL)
			{
			  if (elf_section_data (o)->this_hdr.contents != NULL)
			    ocontents = elf_section_data (o)->this_hdr.contents;
			  else
			    {
			      /* We always cache the section contents.
				 Perhaps, if info->keep_memory is FALSE, we
				 should free them, if we are permitted to.  */

			      if (o->rawsize == 0)
				o->rawsize = o->size;
			      ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
			      if (ocontents == NULL)
				goto error_return;
			      if (!bfd_get_section_contents (abfd, o, ocontents,
							     (file_ptr) 0,
							     o->rawsize))
				goto error_return;
			      elf_section_data (o)->this_hdr.contents = ocontents;
			    }
			}
		      offset = calc_fixup (irelscan->r_addend, 0, sec);
		      irelscan->r_addend -= offset;
		    }
		}
	      else if (ELF32_R_TYPE (irelscan->r_info) == (int) R_MICROBLAZE_64_PCREL)
		{
		  isym = isymbuf + ELF32_R_SYM (irelscan->r_info);

		  /* Look at the reloc only if the value has been resolved.  */
		  if (isym->st_shndx == shndx
		      && (ELF32_ST_TYPE (isym->st_info) == STT_SECTION))
		    {
		      bfd_vma immediate;
		      bfd_vma target_address;

		      if (ocontents == NULL)
			{
			  if (elf_section_data (o)->this_hdr.contents != NULL)
			    ocontents = elf_section_data (o)->this_hdr.contents;
			  else
			    {
			      /* We always cache the section contents.
				 Perhaps, if info->keep_memory is FALSE, we
				 should free them, if we are permitted to.  */
			      if (o->rawsize == 0)
				o->rawsize = o->size;
			      ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
			      if (ocontents == NULL)
				goto error_return;
			      if (!bfd_get_section_contents (abfd, o, ocontents,
							     (file_ptr) 0,
							     o->rawsize))
				goto error_return;
			      elf_section_data (o)->this_hdr.contents = ocontents;
			    }
			}
	  unsigned long instr_hi =  bfd_get_32 (abfd, ocontents
						+ irelscan->r_offset);
	  unsigned long instr_lo =  bfd_get_32 (abfd, ocontents
						+ irelscan->r_offset
						+ INST_WORD_SIZE);
	  immediate = (instr_hi & 0x0000ffff) << 16;
	  immediate |= (instr_lo & 0x0000ffff);
		      target_address = immediate;
		      offset = calc_fixup (target_address, 0, sec);
		      immediate -= offset;
		      irelscan->r_addend -= offset;
	  microblaze_bfd_write_imm_value_64 (abfd, ocontents
					     + irelscan->r_offset, immediate);
		    }
		}
	    }
	}

      /* Adjust the local symbols defined in this section.  */
      isymend = isymbuf + symtab_hdr->sh_info;
      for (isym = isymbuf; isym < isymend; isym++)
	{
	  if (isym->st_shndx == shndx)
	    {
	      isym->st_value -= calc_fixup (isym->st_value, 0, sec);
	      if (isym->st_size)
		isym->st_size -= calc_fixup (isym->st_value, isym->st_size, sec);
	    }
	}

      /* Now adjust the global symbols defined in this section.  */
      isym = isymbuf + symtab_hdr->sh_info;
      symcount =  (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)) - symtab_hdr->sh_info;
      for (sym_index = 0; sym_index < symcount; sym_index++)
	{
	  sym_hash = elf_sym_hashes (abfd)[sym_index];
	  if ((sym_hash->root.type == bfd_link_hash_defined
		  || sym_hash->root.type == bfd_link_hash_defweak)
	      && sym_hash->root.u.def.section == sec)
	    {
	      sym_hash->root.u.def.value -= calc_fixup (sym_hash->root.u.def.value,
							0, sec);
	      if (sym_hash->size)
		sym_hash->size -= calc_fixup (sym_hash->root.u.def.value,
					      sym_hash->size, sec);
	    }
	}

      /* Physically move the code and change the cooked size.  */
      dest = sdata->relax[0].addr;
      for (i = 0; i < sdata->relax_count; i++)
	{
	  size_t len;
	  src = sdata->relax[i].addr + sdata->relax[i].size;
	  len = (sdata->relax[i+1].addr - sdata->relax[i].addr
		 - sdata->relax[i].size);

	  memmove (contents + dest, contents + src, len);
	  sec->size -= sdata->relax[i].size;
	  dest += len;
	}

      elf_section_data (sec)->relocs = internal_relocs;
      free_relocs = NULL;

      elf_section_data (sec)->this_hdr.contents = contents;
      free_contents = NULL;

      symtab_hdr->contents = (bfd_byte *) isymbuf;
    }

  free (free_relocs);
  free_relocs = NULL;

  if (free_contents != NULL)
    {
      if (!link_info->keep_memory)
	free (free_contents);
      else
	/* Cache the section contents for elf_link_input_bfd.  */
	elf_section_data (sec)->this_hdr.contents = contents;
      free_contents = NULL;
    }

  if (sdata->relax_count == 0)
    {
      *again = false;
      free (sdata->relax);
      sdata->relax = NULL;
    }
  else
    *again = true;
  return true;

 error_return:
  free (free_relocs);
  free (free_contents);
  free (sdata->relax);
  sdata->relax = NULL;
  sdata->relax_count = 0;
  return false;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
microblaze_elf_gc_mark_hook (asection *sec,
			     struct bfd_link_info * info,
			     Elf_Internal_Rela * rel,
			     struct elf_link_hash_entry * h,
			     Elf_Internal_Sym * sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_MICROBLAZE_GNU_VTINHERIT:
      case R_MICROBLAZE_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* PIC support.  */

#define PLT_ENTRY_SIZE 16

#define PLT_ENTRY_WORD_0  0xb0000000	      /* "imm 0".  */
#define PLT_ENTRY_WORD_1  0xe9940000	      /* "lwi r12,r20,0" - relocated to lwi r12,r20,func@GOT.  */
#define PLT_ENTRY_WORD_1_NOPIC	0xe9800000    /* "lwi r12,r0,0" - non-PIC object.  */
#define PLT_ENTRY_WORD_2  0x98186000	      /* "brad r12".  */
#define PLT_ENTRY_WORD_3  0x80000000	      /* "nop".  */

static bool
update_local_sym_info (bfd *abfd,
		       Elf_Internal_Shdr *symtab_hdr,
		       unsigned long r_symndx,
		       unsigned int tls_type)
{
  bfd_signed_vma *local_got_refcounts = elf_local_got_refcounts (abfd);
  unsigned char *local_got_tls_masks;

  if (local_got_refcounts == NULL)
    {
      bfd_size_type size = symtab_hdr->sh_info;

      size *= (sizeof (*local_got_refcounts) + sizeof (*local_got_tls_masks));
      local_got_refcounts = bfd_zalloc (abfd, size);
      if (local_got_refcounts == NULL)
	return false;
      elf_local_got_refcounts (abfd) = local_got_refcounts;
    }

  local_got_tls_masks =
	 (unsigned char *) (local_got_refcounts + symtab_hdr->sh_info);
  local_got_tls_masks[r_symndx] |= tls_type;
  local_got_refcounts[r_symndx] += 1;

  return true;
}
/* Look through the relocs for a section during the first phase.  */

static bool
microblaze_elf_check_relocs (bfd * abfd,
			     struct bfd_link_info * info,
			     asection * sec,
			     const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *	rel;
  const Elf_Internal_Rela *	rel_end;
  struct elf32_mb_link_hash_table *htab;
  asection *sreloc = NULL;

  if (bfd_link_relocatable (info))
    return true;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;

  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      struct elf_link_hash_entry * h;
      unsigned long r_symndx;
      unsigned char tls_type = 0;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      switch (r_type)
	{
	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_MICROBLAZE_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_MICROBLAZE_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	  /* This relocation requires .plt entry.  */
	case R_MICROBLAZE_PLT_64:
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  break;

	  /* This relocation requires .got entry.  */
	case R_MICROBLAZE_TLSGD:
	  tls_type |= (TLS_TLS | TLS_GD);
	  goto dogottls;
	case R_MICROBLAZE_TLSLD:
	  tls_type |= (TLS_TLS | TLS_LD);
	  /* Fall through.  */
	dogottls:
	  sec->has_tls_reloc = 1;
	  /* Fall through.  */
	case R_MICROBLAZE_GOT_64:
	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	  if (h != NULL)
	    {
	      h->got.refcount += 1;
	      elf32_mb_hash_entry (h)->tls_mask |= tls_type;
	    }
	  else
	    {
	      if (! update_local_sym_info(abfd, symtab_hdr, r_symndx, tls_type) )
		return false;
	    }
	  break;

	case R_MICROBLAZE_GOTOFF_64:
	case R_MICROBLAZE_GOTOFF_32:
	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	  break;

	case R_MICROBLAZE_64:
	case R_MICROBLAZE_64_PCREL:
	case R_MICROBLAZE_32:
	  {
	    if (h != NULL && !bfd_link_pic (info))
	      {
		/* we may need a copy reloc.  */
		h->non_got_ref = 1;

		/* we may also need a .plt entry.  */
		h->plt.refcount += 1;
		if (ELF32_R_TYPE (rel->r_info) != R_MICROBLAZE_64_PCREL)
		  h->pointer_equality_needed = 1;
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
		 && (r_type != R_MICROBLAZE_64_PCREL
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
		    bfd *dynobj;

		    if (htab->elf.dynobj == NULL)
		      htab->elf.dynobj = abfd;
		    dynobj = htab->elf.dynobj;

		    sreloc = _bfd_elf_make_dynamic_reloc_section (sec, dynobj,
								  2, abfd, 1);
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
		    Elf_Internal_Sym *isym;
		    void *vpp;

		    isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
						  abfd, r_symndx);
		    if (isym == NULL)
		      return false;

		    s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		    if (s == NULL)
		      return false;

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
		if (r_type == R_MICROBLAZE_64_PCREL)
		  p->pc_count += 1;
	      }
	  }
	  break;
	}
    }

  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
microblaze_elf_copy_indirect_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *dir,
				     struct elf_link_hash_entry *ind)
{
  struct elf32_mb_link_hash_entry *edir, *eind;

  edir = (struct elf32_mb_link_hash_entry *) dir;
  eind = (struct elf32_mb_link_hash_entry *) ind;

  edir->tls_mask |= eind->tls_mask;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static bool
microblaze_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				      struct elf_link_hash_entry *h)
{
  struct elf32_mb_link_hash_table *htab;
  asection *s, *srel;
  unsigned int power_of_two;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

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
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC32 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_MICROBLAZE_64_PCREL reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
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

  /* We must generate a R_MICROBLAZE_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
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
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  /* We need to figure out the alignment required for this symbol.  I
     have no idea how ELF linkers handle this.  */
  power_of_two = bfd_log2 (h->size);
  if (power_of_two > 3)
    power_of_two = 3;

  /* Apply the required alignment.  */
  s->size = BFD_ALIGN (s->size, (bfd_size_type) (1 << power_of_two));
  if (power_of_two > s->alignment_power)
    {
      if (!bfd_set_section_alignment (s, power_of_two))
	return false;
    }

  /* Define the symbol as being at this point in the section.  */
  h->root.u.def.section = s;
  h->root.u.def.value = s->size;

  /* Increment the section size to make room for the symbol.  */
  s->size += h->size;
  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void * dat)
{
  struct bfd_link_info *info;
  struct elf32_mb_link_hash_table *htab;
  struct elf32_mb_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) dat;
  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

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

	  /* The first entry in .plt is reserved.  */
	  if (s->size == 0)
	    s->size = PLT_ENTRY_SIZE;

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

  eh = (struct elf32_mb_link_hash_entry *) h;
  if (h->got.refcount > 0)
    {
      unsigned int need;
      asection *s;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      need = 0;
      if ((eh->tls_mask & TLS_TLS) != 0)
	{
	  /* Handle TLS Symbol */
	  if ((eh->tls_mask & TLS_LD) != 0)
	    {
	      if (!eh->elf.def_dynamic)
		/* We'll just use htab->tlsld_got.offset.  This should
		   always be the case.  It's a little odd if we have
		   a local dynamic reloc against a non-local symbol.  */
		htab->tlsld_got.refcount += 1;
	      else
		need += 8;
	    }
	  if ((eh->tls_mask & TLS_GD) != 0)
	    need += 8;
	}
      else
	{
	  /* Regular (non-TLS) symbol */
	  need += 4;
	}
      if (need == 0)
	{
	  h->got.offset = (bfd_vma) -1;
	}
      else
	{
	  s = htab->elf.sgot;
	  h->got.offset = s->size;
	  s->size += need;
	  htab->elf.srelgot->size += need * (sizeof (Elf32_External_Rela) / 4);
	}
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
      if (h->def_regular
	  && (h->forced_local
	      || info->symbolic))
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
      else if (UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	h->dyn_relocs = NULL;
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
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
microblaze_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				      struct bfd_link_info *info)
{
  struct elf32_mb_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  BFD_ASSERT (dynobj != NULL);

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char *lgot_masks;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

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

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      lgot_masks = (unsigned char *) end_local_got;
      s = htab->elf.sgot;
      srel = htab->elf.srelgot;

      for (; local_got < end_local_got; ++local_got, ++lgot_masks)
	{
	  if (*local_got > 0)
	    {
	      unsigned int need = 0;
	      if ((*lgot_masks & TLS_TLS) != 0)
		{
		  if ((*lgot_masks & TLS_GD) != 0)
		    need += 8;
		  if ((*lgot_masks & TLS_LD) != 0)
		    htab->tlsld_got.refcount += 1;
		}
	      else
		need += 4;

	      if (need == 0)
		{
		  *local_got = (bfd_vma) -1;
		}
	      else
		{
		  *local_got = s->size;
		  s->size += need;
		  if (bfd_link_pic (info))
		    srel->size += need * (sizeof (Elf32_External_Rela) / 4);
		}
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);

  if (htab->tlsld_got.refcount > 0)
    {
      htab->tlsld_got.offset = htab->elf.sgot->size;
      htab->elf.sgot->size += 8;
      if (bfd_link_pic (info))
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tlsld_got.offset = (bfd_vma) -1;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Make space for the trailing nop in .plt.  */
      if (htab->elf.splt->size > 0)
	htab->elf.splt->size += 4;
    }

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;
      bool strip = false;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if (startswith (name, ".rela"))
	{
	  if (s->size == 0)
	    {
	      /* If we don't need this section, strip it from the
		 output file.  This is to handle .rela.bss and
		 .rela.plt.  We must create it in
		 create_dynamic_sections, because it must be created
		 before the linker maps input sections to output
		 sections.  The linker does that before
		 adjust_dynamic_symbol is called, and it is that
		 function which decides whether anything needs to go
		 into these sections.  */
	      strip = true;
	    }
	  else
	    {
	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (s != htab->elf.splt
	       && s != htab->elf.sgot
	       && s != htab->elf.sgotplt
	       && s != htab->elf.sdynbss
	       && s != htab->elf.sdynrelro)
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (strip)
	{
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      /* Allocate memory for the section contents.  */
      /* FIXME: This should be a call to bfd_alloc not bfd_zalloc.
	 Unused entries should be reclaimed before the section's contents
	 are written out, but at the moment this does not happen.  Thus in
	 order to prevent writing out garbage, we initialise the section's
	 contents to zero.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL && s->size != 0)
	return false;
    }

  /* ??? Force DF_BIND_NOW?  */
  info->flags |= DF_BIND_NOW;
  return _bfd_elf_add_dynamic_tags (output_bfd, info, true);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
microblaze_elf_finish_dynamic_symbol (bfd *output_bfd,
				      struct bfd_link_info *info,
				      struct elf_link_hash_entry *h,
				      Elf_Internal_Sym *sym)
{
  struct elf32_mb_link_hash_table *htab;
  struct elf32_mb_link_hash_entry *eh = elf32_mb_hash_entry(h);

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *srela;
      asection *sgotplt;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma got_addr;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = htab->elf.splt;
      srela = htab->elf.srelplt;
      sgotplt = htab->elf.sgotplt;
      BFD_ASSERT (splt != NULL && srela != NULL && sgotplt != NULL);

      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1; /* first entry reserved.  */
      got_offset = (plt_index + 3) * 4; /* 3 reserved ???  */
      got_addr = got_offset;

      /* For non-PIC objects we need absolute address of the GOT entry.  */
      if (!bfd_link_pic (info))
	got_addr += sgotplt->output_section->vma + sgotplt->output_offset;

      /* Fill in the entry in the procedure linkage table.  */
      bfd_put_32 (output_bfd, PLT_ENTRY_WORD_0 + ((got_addr >> 16) & 0xffff),
		  splt->contents + h->plt.offset);
      if (bfd_link_pic (info))
	bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1 + (got_addr & 0xffff),
		    splt->contents + h->plt.offset + 4);
      else
	bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1_NOPIC + (got_addr & 0xffff),
		    splt->contents + h->plt.offset + 4);
      bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_2,
		  splt->contents + h->plt.offset + 8);
      bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_3,
		  splt->contents + h->plt.offset + 12);

      /* Any additions to the .got section??? */
      /*      bfd_put_32 (output_bfd,
	      splt->output_section->vma + splt->output_offset + h->plt.offset + 4,
	      sgotplt->contents + got_offset); */

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgotplt->output_section->vma
		       + sgotplt->output_offset
		       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_JUMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Zero the value.  */
	  sym->st_shndx = SHN_UNDEF;
	  sym->st_value = 0;
	}
    }

  /* h->got.refcount to be checked ? */
  if (h->got.offset != (bfd_vma) -1 &&
      ! ((h->got.offset & 1) ||
	  IS_TLS_LD(eh->tls_mask) || IS_TLS_GD(eh->tls_mask)))
    {
      asection *sgot;
      asection *srela;
      bfd_vma offset;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      sgot = htab->elf.sgot;
      srela = htab->elf.srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      offset = (sgot->output_section->vma + sgot->output_offset
		+ (h->got.offset &~ (bfd_vma) 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info)
	  && ((info->symbolic && h->def_regular)
	      || h->dynindx == -1))
	{
	  asection *sec = h->root.u.def.section;
	  bfd_vma value;

	  value = h->root.u.def.value;
	  if (sec->output_section != NULL)
	    /* PR 21180: If the output section is NULL, then the symbol is no
	       longer needed, and in theory the GOT entry is redundant.  But
	       it is too late to change our minds now...  */
	    value += sec->output_section->vma + sec->output_offset;

	  microblaze_elf_output_dynamic_relocation (output_bfd,
						    srela, srela->reloc_count++,
						    /* symindex= */ 0,
						    R_MICROBLAZE_REL, offset,
						    value);
	}
      else
	{
	  microblaze_elf_output_dynamic_relocation (output_bfd,
						    srela, srela->reloc_count++,
						    h->dynindx,
						    R_MICROBLAZE_GLOB_DAT,
						    offset, 0);
	}

      bfd_put_32 (output_bfd, (bfd_vma) 0,
		  sgot->contents + (h->got.offset &~ (bfd_vma) 1));
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbols needs a copy reloc.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark some specially defined symbols as absolute.  */
  if (h == htab->elf.hdynamic
      || h == htab->elf.hgot
      || h == htab->elf.hplt)
    sym->st_shndx = SHN_ABS;

  return true;
}


/* Finish up the dynamic sections.  */

static bool
microblaze_elf_finish_dynamic_sections (bfd *output_bfd,
					struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn, *sgot;
  struct elf32_mb_link_hash_table *htab;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;
	  bool size;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    case DT_PLTGOT:
	      s = htab->elf.sgotplt;
	      size = false;
	      break;

	    case DT_PLTRELSZ:
	      s = htab->elf.srelplt;
	      size = true;
	      break;

	    case DT_JMPREL:
	      s = htab->elf.srelplt;
	      size = false;
	      break;

	    default:
	      continue;
	    }

	  if (s == NULL)
	    dyn.d_un.d_val = 0;
	  else
	    {
	      if (!size)
		dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      else
		dyn.d_un.d_val = s->size;
	    }
	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}

      splt = htab->elf.splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);

      /* Clear the first entry in the procedure linkage table,
	 and put a nop in the last four bytes.  */
      if (splt->size > 0)
	{
	  memset (splt->contents, 0, PLT_ENTRY_SIZE);
	  bfd_put_32 (output_bfd, (bfd_vma) 0x80000000 /* nop.  */,
		      splt->contents + splt->size - 4);

	  if (splt->output_section != bfd_abs_section_ptr)
	    elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
	}
    }

  /* Set the first entry in the global offset table to the address of
     the dynamic section.  */
  sgot = htab->elf.sgotplt;
  if (sgot && sgot->size > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgot->contents);
      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->elf.sgot && htab->elf.sgot->size > 0)
    elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize = 4;

  return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

static bool
microblaze_elf_add_symbol_hook (bfd *abfd,
				struct bfd_link_info *info,
				Elf_Internal_Sym *sym,
				const char **namep ATTRIBUTE_UNUSED,
				flagword *flagsp ATTRIBUTE_UNUSED,
				asection **secp,
				bfd_vma *valp)
{
  if (sym->st_shndx == SHN_COMMON
      && !bfd_link_relocatable (info)
      && sym->st_size <= elf_gp_size (abfd))
    {
      /* Common symbols less than or equal to -G nn bytes are automatically
	 put into .sbss.  */
      *secp = bfd_make_section_old_way (abfd, ".sbss");
      if (*secp == NULL
	  || !bfd_set_section_flags (*secp, SEC_IS_COMMON | SEC_SMALL_DATA))
	return false;

      *valp = sym->st_size;
    }

  return true;
}

#define TARGET_LITTLE_SYM      microblaze_elf32_le_vec
#define TARGET_LITTLE_NAME     "elf32-microblazeel"

#define TARGET_BIG_SYM		microblaze_elf32_vec
#define TARGET_BIG_NAME		"elf32-microblaze"

#define ELF_ARCH		bfd_arch_microblaze
#define ELF_TARGET_ID		MICROBLAZE_ELF_DATA
#define ELF_MACHINE_CODE	EM_MICROBLAZE
#define ELF_MACHINE_ALT1	EM_MICROBLAZE_OLD
#define ELF_MAXPAGESIZE		0x1000
#define elf_info_to_howto	microblaze_elf_info_to_howto
#define elf_info_to_howto_rel	NULL

#define bfd_elf32_bfd_reloc_type_lookup		microblaze_elf_reloc_type_lookup
#define bfd_elf32_bfd_is_local_label_name	microblaze_elf_is_local_label_name
#define bfd_elf32_new_section_hook		microblaze_elf_new_section_hook
#define elf_backend_relocate_section		microblaze_elf_relocate_section
#define bfd_elf32_bfd_relax_section		microblaze_elf_relax_section
#define bfd_elf32_bfd_merge_private_bfd_data	_bfd_generic_verify_endian_match
#define bfd_elf32_bfd_reloc_name_lookup		microblaze_elf_reloc_name_lookup

#define elf_backend_gc_mark_hook		microblaze_elf_gc_mark_hook
#define elf_backend_check_relocs		microblaze_elf_check_relocs
#define elf_backend_copy_indirect_symbol	microblaze_elf_copy_indirect_symbol
#define bfd_elf32_bfd_link_hash_table_create	microblaze_elf_link_hash_table_create
#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_want_got_plt		1
#define elf_backend_plt_readonly		1
#define elf_backend_got_header_size		12
#define elf_backend_want_dynrelro		1
#define elf_backend_rela_normal			1
#define elf_backend_dtrel_excludes_plt		1

#define elf_backend_adjust_dynamic_symbol	microblaze_elf_adjust_dynamic_symbol
#define elf_backend_create_dynamic_sections	_bfd_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections	microblaze_elf_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol	microblaze_elf_finish_dynamic_symbol
#define elf_backend_size_dynamic_sections	microblaze_elf_size_dynamic_sections
#define elf_backend_add_symbol_hook		microblaze_elf_add_symbol_hook

#include "elf32-target.h"
