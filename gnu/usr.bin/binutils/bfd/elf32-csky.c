/* 32-bit ELF support for C-SKY.
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by C-SKY Microsystems and Mentor Graphics.

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
#include "elf/csky.h"
#include "opcode/csky.h"
#include <assert.h>
#include "libiberty.h"
#include "elf32-csky.h"

/* Data structures used for merging different arch variants.
   V1 (510/610) and V2 (8xx) processors are incompatible, but
   we can merge wthin each family.  */

enum merge_class
{
  CSKY_V1,
  CSKY_V2
};

typedef const struct csky_arch_for_merge
{
  const char *name;
  const unsigned long arch_eflag;
  /* The files can merge only if they are in same class.  */
  enum merge_class class;
  /* When input files have different levels,
     the target sets arch_eflag to the largest level file's arch_eflag.  */
  unsigned int class_level;
  /* Control whether to print warning when merging with different arch.  */
  unsigned int do_warning;
} csky_arch_for_merge;

static csky_arch_for_merge csky_archs[] =
{
  /* 510 and 610 merge to 610 without warning.  */
  { "ck510",  CSKY_ARCH_510,  CSKY_V1,  0, 0},
  { "ck610",  CSKY_ARCH_610,  CSKY_V1,  1, 0},
  /* 801, 802, 803, 807, 810 merge to largest one.  */
  { "ck801",  CSKY_ARCH_801,  CSKY_V2,  0, 1},
  { "ck802",  CSKY_ARCH_802,  CSKY_V2,  1, 1},
  { "ck803",  CSKY_ARCH_803,  CSKY_V2,  2, 1},
  { "ck807",  CSKY_ARCH_807,  CSKY_V2,  3, 1},
  { "ck810",  CSKY_ARCH_810,  CSKY_V2,  4, 1},
  { "ck860",  CSKY_ARCH_860,  CSKY_V2,  5, 1},
  { NULL, 0, 0, 0, 0}
};

/* Return the ARCH bits out of ABFD.  */
#define bfd_csky_arch(abfd) \
  (elf_elfheader (abfd)->e_flags & CSKY_ARCH_MASK)

/* Return the ABI bits out of ABFD.  */
#define bfd_csky_abi(abfd) \
  (elf_elfheader (abfd)->e_flags & CSKY_ABI_MASK)


/* The index of a howto-item is implicitly equal to
   the corresponding Relocation Type Encoding.  */
static reloc_howto_type csky_elf_howto_table[] =
{
  /* 0 */
  HOWTO (R_CKCORE_NONE,               /* type */
	 0,                           /* rightshift */
	 0,                           /* size */
	 0,                           /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_NONE",             /* name */
	 false,                       /* partial_inplace */
	 0,                           /* src_mask */
	 0,                           /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 1.  */
  HOWTO (R_CKCORE_ADDR32,             /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_ADDR32",           /* name */
	 false,                       /* partial_inplace */
	 0,                           /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 2: Only for csky v1.  */
  HOWTO (R_CKCORE_PCREL_IMM8BY4,      /* type */
	 2,                           /* rightshift */
	 2,                           /* size */
	 8,                           /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_PCREL_IMM8BY4",    /* name */
	 false,                       /* partial_inplace */
	 0xff,                        /* src_mask */
	 0xff,                        /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 3: Only for csky v1.  */
  HOWTO (R_CKCORE_PCREL_IMM11BY2,     /* type */
	 1,                           /* rightshift */
	 2,                           /* size */
	 11,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_IMM11BY2",   /* name */
	 false,                       /* partial_inplace */
	 0x7ff,                       /* src_mask */
	 0x7ff,                       /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 4: DELETED.  */
  HOWTO (R_CKCORE_PCREL_IMM4BY2,0,0,0,0,0,0,0,"R_CKCORE_PCREL_IMM4BY2",0,0,0,0),

  /* 5.  */
  HOWTO (R_CKCORE_PCREL32,            /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL32",          /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 6: Only for csky v1.  */
  HOWTO (R_CKCORE_PCREL_JSR_IMM11BY2, /* type */
	 1,                           /* rightshift */
	 2,                           /* size */
	 11,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_JSR_IMM11BY2", /* name */
	 false,                       /* partial_inplace */
	 0x7ff,                       /* src_mask */
	 0x7ff,                       /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 7: GNU extension to record C++ vtable member usage.  */
  HOWTO (R_CKCORE_GNU_VTENTRY,        /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 0,                           /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_CKCORE_GNU_VTENTRY",      /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x0,                         /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 8: GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_CKCORE_GNU_VTINHERIT,      /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 0,                           /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_GNU_VTINHERIT",    /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x0,                         /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 9.  */
  HOWTO (R_CKCORE_RELATIVE,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_RELATIVE",         /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 10: None.  */
  /* FIXME:  It is a bug that copy relocations are not implemented.  */
  HOWTO (R_CKCORE_COPY,               /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_COPY",             /* name */
	 true,                        /* partial_inplace */
	 0xffffffff,                  /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 11: None.  */
  HOWTO (R_CKCORE_GLOB_DAT,0,0,0,0,0,0,0,"R_CKCORE_GLOB_DAT",0,0,0,0),

  /* 12: None.  */
  HOWTO (R_CKCORE_JUMP_SLOT,0,0,0,0,0,0,0,"R_CKCORE_JUMP_SLOT",0,0,0,0),

  /* 13.  */
  HOWTO (R_CKCORE_GOTOFF,             /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTOFF",           /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffffl,                 /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 14.  */
  HOWTO (R_CKCORE_GOTPC,              /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTPC",            /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 15.  */
  HOWTO (R_CKCORE_GOT32,              /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOT32",            /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 16.  */
  HOWTO (R_CKCORE_PLT32,              /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PLT32",            /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 17: None.  */
  HOWTO (R_CKCORE_ADDRGOT,0,0,0,0,0,0,0,"R_CKCORE_ADDRGOT",0,0,0,0),

  /* 18: None.  */
  HOWTO (R_CKCORE_ADDRPLT,0,0,0,0,0,0,0,"R_CKCORE_ADDRPLT",0,0,0,0),

  /* 19: Only for csky v2.  */
  HOWTO (R_CKCORE_PCREL_IMM26BY2,     /* type */
	 1,                           /* rightshift */
	 4,                           /* size */
	 26,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_IMM26BY2",   /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ffffff,                   /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 20: Only for csky v2.  */
  HOWTO (R_CKCORE_PCREL_IMM16BY2,     /* type */
         1,                           /* rightshift */
         4,                           /* size */
         16,                          /* bitsize */
         true,                        /* pc_relative */
         0,                           /* bitpos */
         complain_overflow_signed,    /* complain_on_overflow */
         bfd_elf_generic_reloc,       /* special_function */
         "R_CKCORE_PCREL_IMM16BY2",   /* name */
         false,                       /* partial_inplace */
         0x0,                         /* src_mask */
         0xffff,                      /* dst_mask */
         true),                       /* pcrel_offset */

  /* 21: Only for csky v2.  */
  HOWTO (R_CKCORE_PCREL_IMM16BY4,     /* type */
         2,                           /* rightshift */
         4,                           /* size */
         16,                          /* bitsize */
         true,                        /* pc_relative */
         0,                           /* bitpos */
         complain_overflow_bitfield,  /* complain_on_overflow */
         bfd_elf_generic_reloc,       /* special_function */
         "R_CKCORE_PCREL_IMM16BY4",   /* name */
         false,                       /* partial_inplace */
         0xffff0000,                  /* src_mask */
         0xffff,                      /* dst_mask */
         true),                       /* pcrel_offset */

  /* 22: Only for csky v2.  */
  HOWTO (R_CKCORE_PCREL_IMM10BY2,     /* type */
	 1,                           /* rightshift */
	 2,                           /* size */
	 10,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_IMM10BY2",   /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ff,                       /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 23: Only for csky v2.  */
  HOWTO (R_CKCORE_PCREL_IMM10BY4,     /* type */
         2,                           /* rightshift */
         4,                           /* size */
         10,                          /* bitsize */
         true,                        /* pc_relative */
         0,                           /* bitpos */
         complain_overflow_bitfield,  /* complain_on_overflow */
         bfd_elf_generic_reloc,       /* special_function */
         "R_CKCORE_PCREL_IMM10BY4",   /* name */
         false,                       /* partial_inplace */
         0x0,                         /* src_mask */
         0x3ff,                       /* dst_mask */
         true),                       /* pcrel_offset */

  /* 24: Only for csky v2.  */
  HOWTO (R_CKCORE_ADDR_HI16,          /* type */
	 16,                          /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_ADDR_HI16",        /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 25.  */
  HOWTO (R_CKCORE_ADDR_LO16,          /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_ADDR_LO16",        /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 26.  */
  HOWTO (R_CKCORE_GOTPC_HI16,         /* type */
	 16,                          /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTPC_HI16",       /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 27.  */
  HOWTO (R_CKCORE_GOTPC_LO16,         /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTPC_LO16",       /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 28.  */
  HOWTO (R_CKCORE_GOTOFF_HI16,        /* type */
	 16,                          /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTOFF_HI16",      /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 29.  */
  HOWTO (R_CKCORE_GOTOFF_LO16,        /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTOFF_LO16",      /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 30.  */
  HOWTO (R_CKCORE_GOT12,              /* type */
	 2,                           /* rightshift */
	 4,                           /* size */
	 12,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOT12",            /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xfff,                       /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 31.  */
  HOWTO (R_CKCORE_GOT_HI16,           /* type */
	 16,                          /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOT_HI16",         /* name */
	 true,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 32.  */
  HOWTO (R_CKCORE_GOT_LO16,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOT_LO16",         /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 33.  */
  HOWTO (R_CKCORE_PLT12,              /* type */
	 2,                           /* rightshift */
	 4,                           /* size */
	 12,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PLT12",            /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xfff,                       /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 34.  */
  HOWTO (R_CKCORE_PLT_HI16,           /* type */
	 16,                          /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PLT_HI16",         /* name */
	 true,                        /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 35.  */
  HOWTO (R_CKCORE_PLT_LO16,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PLT_LO16",         /* name */
	 true,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 36: None.  */
  HOWTO (R_CKCORE_ADDRGOT_HI16,0,0,0,0,0,0,0,"R_CKCORE_",0,0,0,0),

  /* 37: None.  */
  HOWTO (R_CKCORE_ADDRGOT_LO16,0,0,0,0,0,0,0,"R_CKCORE_",0,0,0,0),

  /* 38: None.  */
  HOWTO (R_CKCORE_ADDRPLT_HI16,0,0,0,0,0,0,0,"R_CKCORE_",0,0,0,0),

  /* 39: None.  */
  HOWTO (R_CKCORE_ADDRPLT_LO16,0,0,0,0,0,0,0,"R_CKCORE_",0,0,0,0),

  /* 40.  */
  HOWTO (R_CKCORE_PCREL_JSR_IMM26BY2, /* type */
	 1,                           /* rightshift */
	 4,                           /* size */
	 26,                          /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_JSR_IMM26BY2", /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ffffff,                   /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 41.  */
  HOWTO (R_CKCORE_TOFFSET_LO16,       /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_unsigned,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_TOFFSET_LO16",     /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 42.  */
  HOWTO (R_CKCORE_DOFFSET_LO16,       /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 16,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_unsigned,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_DOFFSET_LO16",     /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 43.  */
  HOWTO (R_CKCORE_PCREL_IMM18BY2,     /* type */
         1,                           /* rightshift */
         4,                           /* size */
         18,                          /* bitsize */
         true,                        /* pc_relative */
         0,                           /* bitpos */
         complain_overflow_signed,    /* complain_on_overflow */
         bfd_elf_generic_reloc,       /* special_function */
         "R_CKCORE_PCREL_IMM18BY2",   /* name */
         false,                       /* partial_inplace */
         0x0,                         /* src_mask */
         0x3ffff,                     /* dst_mask */
         true),                       /* pcrel_offset */

  /* 44.  */
  HOWTO (R_CKCORE_DOFFSET_IMM18,      /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_unsigned,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_DOFFSET_IMM18",    /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 45.  */
  HOWTO (R_CKCORE_DOFFSET_IMM18BY2,   /* type */
	 1,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_unsigned,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_DOFFSET_IMM18BY2", /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 46.  */
  HOWTO (R_CKCORE_DOFFSET_IMM18BY4,   /* type */
	 2,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_unsigned,  /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_DOFFSET_IMM18BY4", /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 47.  */
  HOWTO (R_CKCORE_GOTOFF_IMM18,       /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOTOFF_IMM18",     /* name */
	 true,                        /* partial_inplace */
	 0xfffc,                      /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 48.  */
  HOWTO (R_CKCORE_GOT_IMM18BY4,       /* type */
	 2,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_GOT_IMM18BY4",     /* name */
	 true,                        /* partial_inplace */
	 0xfffc,                      /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 49.  */
  HOWTO (R_CKCORE_PLT_IMM18BY4,       /* type */
	 2,                           /* rightshift */
	 4,                           /* size */
	 18,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PLT_IMM18BY4",     /* name */
	 true,                        /* partial_inplace */
	 0xfffc,                      /* src_mask */
	 0x3ffff,                     /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 50: for lrw16.  */
  HOWTO (R_CKCORE_PCREL_IMM7BY4,      /* type */
	 2,                           /* rightshift */
	 2,                           /* size */
	 7,                           /* bitsize */
	 true,                        /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_bitfield,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_IMM7BY4",    /* name */
	 false,                       /* partial_inplace */
	 0xec1f,                      /* src_mask */
	 0x31f,                       /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 51: for static nptl.  */
  HOWTO (R_CKCORE_TLS_LE32,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_TLS_LE32",         /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 52: for static nptl.  */
  HOWTO (R_CKCORE_TLS_IE32,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_TLS_IE32",         /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 53: for pic nptl.  */
  HOWTO (R_CKCORE_TLS_GD32,           /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_TLS_GD32",         /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 54: for pic nptl.  */
  HOWTO (R_CKCORE_TLS_LDM32,          /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_TLS_LDM32",        /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 55: for pic nptl.  */
  HOWTO (R_CKCORE_TLS_LDO32,          /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_TLS_LDO32",        /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xffffffff,                  /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 56: for linker.  */
  HOWTO (R_CKCORE_TLS_DTPMOD32,0,0,0,0,0,0,0,"R_CKCORE_TLS_DTPMOD32",0,0,0,0),

  /* 57: for linker.  */
  HOWTO (R_CKCORE_TLS_DTPOFF32,0,0,0,0,0,0,0,"R_CKCORE_TLS_DTPOFF32",0,0,0,0),

  /* 58: for linker.  */
  HOWTO (R_CKCORE_TLS_TPOFF32,0,0,0,0,0,0,0,"R_CKCORE_TLS_TPOFF32",0,0,0,0),

  /* 59: for ck807f.  */
  HOWTO (R_CKCORE_PCREL_FLRW_IMM8BY4, /* type */
         2,                           /* rightshift */
         4,                           /* size */
         8,                           /* bitsize */
         true,                        /* pc_relative */
         0,                           /* bitpos */
         complain_overflow_bitfield,  /* complain_on_overflow */
         bfd_elf_generic_reloc,       /* special_function */
         "R_CKCORE_PCREL_FLRW_IMM8BY4",/* name */
         false,                       /* partial_inplace */
         0xfe1fff0f,                  /* src_mask */
         0x1e000f0,                   /* dst_mask */
         true),                       /* pcrel_offset */

  /* 60: for 810 not to generate jsri.  */
  HOWTO (R_CKCORE_NOJSRI,             /* type */
	 0,                           /* rightshift */
	 4,                           /* size */
	 32,                          /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_NOJSRI",           /* name */
	 false,                       /* partial_inplace */
	 0xffff,                      /* src_mask */
	 0xffff,                      /* dst_mask */
	 false),                      /* pcrel_offset */

  /* 61: for callgraph.  */
  HOWTO (R_CKCORE_CALLGRAPH,          /* type */
	 0,                           /* rightshift */
	 0,                           /* size */
	 0,                           /* bitsize */
	 false,                       /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_dont,      /* complain_on_overflow */
	 NULL,                        /* special_function */
	 "R_CKCORE_CALLGRAPH",        /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0x0,                         /* dst_mask */
	 true),                       /* pcrel_offset */

  /* 62: IRELATIVE*/
  HOWTO (R_CKCORE_IRELATIVE,0,0,0,0,0,0,0,"R_CKCORE_IRELATIVE",0,0,0,0),

  /* 63: for bloop instruction */
  HOWTO (R_CKCORE_PCREL_BLOOP_IMM4BY4, /* type */
	 1,                           /* rightshift */
	 4,                           /* size */
	 4,                           /* bitsize */
	 1,                           /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_BLOOP_IMM4BY4", /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xf,                         /* dst_mask */
	 true),                       /* pcrel_offset */
  /* 64: for bloop instruction */
  HOWTO (R_CKCORE_PCREL_BLOOP_IMM12BY4, /* type */
	 1,                           /* rightshift */
	 4,                           /* size */
	 12,                          /* bitsize */
	 1,                           /* pc_relative */
	 0,                           /* bitpos */
	 complain_overflow_signed,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,       /* special_function */
	 "R_CKCORE_PCREL_BLOOP_IMM12BY4", /* name */
	 false,                       /* partial_inplace */
	 0x0,                         /* src_mask */
	 0xfff,                       /* dst_mask */
	 true),                       /* pcrel_offset */


};


/* Whether GOT overflow checking is needed.  */
static int check_got_overflow = 0;

/* Whether the target 32 bits is forced so that the high
   16 bits is at the low address.  */
static int need_reverse_bits;

/* Used for relaxation.  See csky_relocate_contents.  */
static bfd_vma read_content_substitute;

/* NOTICE!
   The way the following two look-up functions work demands
   that BFD_RELOC_CKCORE_xxx are defined contiguously.  */

static reloc_howto_type *
csky_elf_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
			    bfd_reloc_code_real_type code)
{
  int csky_code = code - BFD_RELOC_CKCORE_NONE;

  if (csky_code < 0 || csky_code >= R_CKCORE_MAX)
    {
      switch (code)
	{
	case BFD_RELOC_NONE:
	  csky_code = R_CKCORE_NONE;
	  break;
	case BFD_RELOC_32:
	  csky_code = R_CKCORE_ADDR32;
	  break;
	case BFD_RELOC_32_PCREL:
	  csky_code = R_CKCORE_PCREL32;
	  break;
	case BFD_RELOC_VTABLE_INHERIT:
	  csky_code = R_CKCORE_GNU_VTINHERIT;
	  break;
	case BFD_RELOC_VTABLE_ENTRY:
	  csky_code = R_CKCORE_GNU_VTENTRY;
	  break;
	case BFD_RELOC_RVA:
	  csky_code = R_CKCORE_RELATIVE;
	  break;
	default:
	  return (reloc_howto_type *)NULL;
	}
    }
  /* Note: when adding csky bfd reloc types in bfd-in2.h
     and csky elf reloc types in elf/csky.h,
     the order of the two reloc type tables should be consistent.  */
  return &csky_elf_howto_table[csky_code];
}

static reloc_howto_type *
csky_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;
  for (i = 0; i < R_CKCORE_MAX; i++)
    if (strcasecmp (csky_elf_howto_table[i].name, r_name) == 0)
      return &csky_elf_howto_table[i];
  return NULL;
}

static reloc_howto_type *
elf32_csky_howto_from_type (unsigned int r_type)
{
  if (r_type < R_CKCORE_MAX)
    return &csky_elf_howto_table[r_type];
  else
    return NULL;
}

static bool
csky_elf_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			arelent *cache_ptr,
			Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = elf32_csky_howto_from_type (r_type);
  if (cache_ptr->howto == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

/* The Global Offset Table max size.  */
#define GOT_MAX_SIZE 0xFFFF8

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER     "/usr/lib/ld.so.1"

/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE      12
#define PLT_ENTRY_SIZE_P    16

/* The first entry in a procedure linkage table looks like
   this.  It is set up so that any shared library function that is
   called before the relocation has been set up calls the dynamic
   linker first.  */
static const bfd_vma csky_elf_plt_entry_v2[PLT_ENTRY_SIZE / 4] =
{
  0xd99c2002,  /* ldw r12, (gb, 8)  */
  0xea0d0000,  /* movi r13,offset   */
  0xe8cc0000   /* jmp r12           */
};

static const bfd_vma csky_elf_plt_entry_v1[PLT_ENTRY_SIZE / 2 ] =
{
  0x25f0,  /* subi r0, 32       */
  0x9200,  /* stw r2, (r0, 0)   */
  0x9310,  /* stw r3, (r0, 4)   */
  0x822e,  /* ldw r2, (gb, 8)   */
  0x7301,  /* lrw r3, #offset   */
  0x00c2,  /* jmp r2            */
};

/* Branch stub support.  */

enum stub_insn_type
{
  INSN16,
  INSN32,
  DATA_TYPE
};

bool use_branch_stub = true;
typedef struct
{
  bfd_vma data;
  enum stub_insn_type type;
  unsigned int r_type;
  int reloc_addend;
} insn_sequence;

static const insn_sequence elf32_csky_stub_long_branch[] =
{
  {0xea8d0002, INSN32,    R_CKCORE_NONE,   0x0},   /* lrw t1,[pc+8] */
  {0x7834,     INSN16,    R_CKCORE_NONE,   0x0},   /* jmp t1 */
  {0x6c03,     INSN16,    R_CKCORE_NONE,   0x0},   /* nop */
  {0x0,        DATA_TYPE, R_CKCORE_ADDR32, 0x0}    /* .long addr */
};

static const insn_sequence elf32_csky_stub_long_branch_jmpi[] =
{
  {0xeac00001, INSN32,    R_CKCORE_NONE,   0x0},   /* jmpi [pc+4] */
  {0x0,        DATA_TYPE, R_CKCORE_ADDR32, 0x0}    /* .long addr */
};

/* The bsr instruction offset limit.  */
#define BSR_MAX_FWD_BRANCH_OFFSET       (((1 << 25) - 1) << 1)
#define BSR_MAX_BWD_BRANCH_OFFSET       (-(1 << 26))

#define STUB_SUFFIX ".stub"
#define STUB_ENTRY_NAME "__%s_veneer"

/* One entry per long/short branch stub defined above.  */
#define DEF_STUBS \
  DEF_STUB(long_branch) \
  DEF_STUB(long_branch_jmpi)

#define DEF_STUB(x) csky_stub_##x,
enum elf32_csky_stub_type
{
  csky_stub_none,
  DEF_STUBS
};
#undef DEF_STUB

typedef struct
{
  const insn_sequence* template_sequence;
  int template_size;
} stub_def;

#define DEF_STUB(x) {elf32_csky_stub_##x, ARRAY_SIZE(elf32_csky_stub_##x)},
static const stub_def stub_definitions[] = {
  {NULL, 0},
  DEF_STUBS
};

/* The size of the thread control block.  */
#define TCB_SIZE        8

struct csky_elf_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;
};

#define csky_elf_local_got_tls_type(bfd) \
  (csky_elf_tdata (bfd)->local_got_tls_type)

#define csky_elf_tdata(bfd) \
  ((struct csky_elf_obj_tdata *) (bfd)->tdata.any)

struct elf32_csky_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump).  */
  bfd_vma target_value;
  asection *target_section;

    /* Offset to apply to relocation referencing target_value.  */
  bfd_vma target_addend;

  /* The stub type.  */
  enum elf32_csky_stub_type stub_type;
  /* Its encoding size in bytes.  */
  int stub_size;
  /* Its template.  */
  const insn_sequence *stub_template;
  /* The size of the template (number of entries).  */
  int stub_template_size;

  /* The symbol table entry, if any, that this was derived from.  */
  struct csky_elf_link_hash_entry *h;

  /* Destination symbol type.  */
  unsigned char st_type;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;

  /* The name for the local symbol at the start of this stub.  The
     stub name in the hash table has to be unique; this does not, so
     it can be friendlier.  */
  char *output_name;
};

#define csky_stub_hash_lookup(table, string, create, copy) \
  ((struct elf32_csky_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* C-SKY ELF linker hash entry.  */
struct csky_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;
  int plt_refcount;
  /* For sub jsri2bsr relocs count.  */
  int jsri2bsr_refcount;

#define GOT_UNKNOWN     0
#define GOT_NORMAL      1
#define GOT_TLS_GD      2
#define GOT_TLS_IE      4

  unsigned char tls_type;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf32_csky_stub_hash_entry *stub_cache;
};

/* Traverse an C-SKY ELF linker hash table.  */
#define csky_elf_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),		\
    (info)))

/* Get the C-SKY ELF linker hash table from a link_info structure.  */
#define csky_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == CSKY_ELF_DATA)		\
   ? (struct csky_elf_link_hash_table *) (p)->hash : NULL)

#define csky_elf_hash_entry(ent)  ((struct csky_elf_link_hash_entry*)(ent))

/* Array to keep track of which stub sections have been created, and
   information on stub grouping.  */
struct map_stub
{
  /* This is the section to which stubs in the group will be
     attached.  */
  asection *link_sec;
  /* The stub section.  */
  asection *stub_sec;
};

/* C-SKY ELF linker hash table.  */
struct csky_elf_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Data for R_CKCORE_TLS_LDM32 relocations.  */
  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tls_ldm_got;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);

  /* Array to keep track of which stub sections have been created, and
   * information on stub grouping.  */
  struct map_stub *stub_group;

  /* Number of elements in stub_group.  */
  unsigned int top_id;

  /* Assorted information used by elf32_csky_size_stubs.  */
  unsigned int bfd_count;
  unsigned int top_index;
  asection **input_list;
};

/* We can't change vectors in the bfd target which will apply to
   data sections, however we only do this to the text sections.  */

static bfd_vma
csky_get_insn_32 (bfd *input_bfd,
		  bfd_byte *location)
{
  if (bfd_big_endian (input_bfd))
    return bfd_get_32 (input_bfd, location);
  else
    return (bfd_get_16 (input_bfd, location) << 16
	    | bfd_get_16 (input_bfd, location + 2));
}

static void
csky_put_insn_32 (bfd *input_bfd,
		  bfd_vma x,
		  bfd_byte *location)
{
  if (bfd_big_endian (input_bfd))
    bfd_put_32 (input_bfd, x, location);
  else
    {
      bfd_put_16 (input_bfd, x >> 16, location);
      bfd_put_16 (input_bfd, x & 0xffff, location + 2);
    }
}

/* Find or create a stub section.  Returns a pointer to the stub section, and
   the section to which the stub section will be attached (in *LINK_SEC_P).
   LINK_SEC_P may be NULL.  */

static asection *
elf32_csky_create_or_find_stub_sec (asection **link_sec_p, asection *section,
				    struct csky_elf_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;

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

  if (link_sec_p)
    *link_sec_p = link_sec;

  return stub_sec;
}

/* Build a name for an entry in the stub hash table.  */

static char *
elf32_csky_stub_name (const asection *input_section,
		      const asection *sym_sec,
		      const struct csky_elf_link_hash_entry *hash,
		      const Elf_Internal_Rela *rel)
{
  char *stub_name;
  bfd_size_type len;

  if (hash)
    {
      len = 8 + 1 + strlen (hash->elf.root.root.string) + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	sprintf (stub_name, "%08x_%s+%x",
		 input_section->id & 0xffffffff,
		 hash->elf.root.root.string,
		 (int) rel->r_addend & 0xffffffff);
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	sprintf (stub_name, "%08x_%x:%x+%x",
		 input_section->id & 0xffffffff,
		 sym_sec->id & 0xffffffff,
		 (int) ELF32_R_SYM (rel->r_info) & 0xffffffff,
		 (int) rel->r_addend & 0xffffffff);
    }

  return stub_name;
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf32_csky_stub_type
csky_type_of_stub (struct bfd_link_info *info,
		   asection *input_sec,
		   const Elf_Internal_Rela *rel,
		   unsigned char st_type,
		   struct csky_elf_link_hash_entry *hash,
		   bfd_vma destination,
		   asection *sym_sec ATTRIBUTE_UNUSED,
		   bfd *input_bfd ATTRIBUTE_UNUSED,
		   const char *name ATTRIBUTE_UNUSED)
{
  bfd_vma location;
  bfd_signed_vma branch_offset;
  unsigned int r_type;
  enum elf32_csky_stub_type stub_type = csky_stub_none;
  struct elf_link_hash_entry * h = &hash->elf;

  /* We don't know the actual type of destination in case it is of
     type STT_SECTION: give up.  */
  if (st_type == STT_SECTION)
    return stub_type;

  location = (input_sec->output_offset
	      + input_sec->output_section->vma
	      + rel->r_offset);

  branch_offset = (bfd_signed_vma)(destination - location);
  r_type = ELF32_R_TYPE (rel->r_info);
  if (r_type == R_CKCORE_PCREL_IMM26BY2
      && ((h != NULL
	   && ((h->def_dynamic && !h->def_regular)
	       || (bfd_link_pic (info)
		   && h->root.type == bfd_link_hash_defweak)))
	  || branch_offset > BSR_MAX_FWD_BRANCH_OFFSET
	  || branch_offset < BSR_MAX_BWD_BRANCH_OFFSET))
    {
      if (bfd_csky_arch (info->output_bfd) == CSKY_ARCH_810
	  || bfd_csky_arch (info->output_bfd) ==  CSKY_ARCH_807)
	stub_type = csky_stub_long_branch_jmpi;
      else
	stub_type = csky_stub_long_branch;
    }

  return stub_type;
}

/* Create an entry in an C-SKY ELF linker hash table.  */

static struct bfd_hash_entry *
csky_elf_link_hash_newfunc (struct bfd_hash_entry * entry,
			    struct bfd_hash_table * table,
			    const char * string)
{
  struct csky_elf_link_hash_entry * ret =
      (struct csky_elf_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    {
      ret = (struct csky_elf_link_hash_entry *)
	bfd_hash_allocate (table,
			   sizeof (struct csky_elf_link_hash_entry));
      if (ret == NULL)
	return (struct bfd_hash_entry *) ret;
    }

  /* Call the allocation method of the superclass.  */
  ret = ((struct csky_elf_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *)ret,
				     table, string));
  if (ret != NULL)
    {
      struct csky_elf_link_hash_entry *eh;

      eh = (struct csky_elf_link_hash_entry *) ret;
      eh->plt_refcount = 0;
      eh->jsri2bsr_refcount = 0;
      eh->tls_type = GOT_NORMAL;
      ret->stub_cache = NULL;
    }

  return (struct bfd_hash_entry *) ret;
}

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
      entry = ((struct bfd_hash_entry *)
	       bfd_hash_allocate (table,
				  sizeof (struct elf32_csky_stub_hash_entry)));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_csky_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf32_csky_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->target_addend = 0;
      eh->stub_type = csky_stub_none;
      eh->stub_size = 0;
      eh->stub_template = NULL;
      eh->stub_template_size = -1;
      eh->h = NULL;
      eh->id_sec = NULL;
      eh->output_name = NULL;
    }

  return entry;
}

/* Free the derived linker hash table.  */

static void
csky_elf_link_hash_table_free (bfd *obfd)
{
  struct csky_elf_link_hash_table *ret
    = (struct csky_elf_link_hash_table *) obfd->link.hash;

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an CSKY elf linker hash table.  */

static struct bfd_link_hash_table *
csky_elf_link_hash_table_create (bfd *abfd)
{
  struct csky_elf_link_hash_table *ret;
  size_t amt = sizeof (struct csky_elf_link_hash_table);

  ret = (struct csky_elf_link_hash_table*) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd,
				      csky_elf_link_hash_newfunc,
				      sizeof (struct csky_elf_link_hash_entry),
				      CSKY_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf32_csky_stub_hash_entry)))
    {
      free (ret);
      return NULL;
    }
  ret->elf.root.hash_table_free = csky_elf_link_hash_table_free;
  return &ret->elf.root;
}

static bool
csky_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct csky_elf_obj_tdata),
				  CSKY_ELF_DATA);
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
csky_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h)
{
  struct csky_elf_link_hash_entry *eh;
  struct csky_elf_link_hash_table *htab;
  asection *srel;
  asection *s;
  eh = (struct csky_elf_link_hash_entry *)h;
  if (eh == NULL)
    return false;

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Clear jsri2bsr_refcount, if creating shared library files.  */
  if (bfd_link_pic (info) && eh->jsri2bsr_refcount > 0)
    eh->jsri2bsr_refcount = 0;

  /* If there is a function, put it in the procedure linkage table. We
     will fill in the contents of the procedure linkage table later.  */
  if (h->needs_plt)
    {
      /* Calls to STT_GNU_IFUNC symbols always use a PLT, even if the
	 symbol binds locally.  */
      if (h->plt.refcount <= 0
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_CALLS_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))

	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC32 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	  if (h->got.refcount == 0)
	    h->got.refcount += 1;
	}
      else if (h->got.refcount != 0)
	{
	  h->got.refcount -= eh->plt_refcount;
	  eh->plt_refcount = 0;
	}
      return true;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_CKCORE_PC32 or similar reloc to a non-function
       sym in check_relocs.  We can't decide accurately between function
       and non-function syms in check_relocs; objects loaded later in
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

  /* If there are no non-GOT references, we do not need a copy
     relocation.  */
  if (!h->non_got_ref)
    return true;

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info) || htab->elf.is_relocatable_executable)
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
  /* We must generate a R_CKCORE_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
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
  if (info->nocopyreloc == 0
      && (h->root.u.def.section->flags & SEC_ALLOC) != 0
      && h->size != 0
      && srel != NULL
      && s != NULL)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
      return _bfd_elf_adjust_dynamic_copy (info, h, s);
    }

  h->non_got_ref = 0;
  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
csky_allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct csky_elf_link_hash_table *htab;
  struct csky_elf_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  /* For indirect case, such as _ZdlPv to _ZdlPv@@GLIBCXX_3.4.  */
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;


  info = (struct bfd_link_info *) inf;
  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;
  /*TODO: how to deal with weak symbol relocs.  */
  if ((htab->elf.dynamic_sections_created || h->type == STT_GNU_IFUNC)
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local
	  && h->root.type == bfd_link_hash_undefweak
	  && ! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
      if (bfd_link_pic (info) || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *splt = htab->elf.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (splt->size == 0)
	    {
	      if (bfd_csky_abi (info->output_bfd) ==  CSKY_ABI_V1)
		splt->size += PLT_ENTRY_SIZE_P;
	      else
		splt->size += PLT_ENTRY_SIZE;
	    }
	  h->plt.offset = splt->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (!bfd_link_pic (info) && !h->def_regular)
	    {
	      h->root.u.def.section = splt;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  if (bfd_csky_abi (info->output_bfd) ==  CSKY_ABI_V1)
	    splt->size += PLT_ENTRY_SIZE_P;
	  else
	    splt->size += PLT_ENTRY_SIZE;
	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->elf.srelplt->size += sizeof (Elf32_External_Rela);

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->elf.sgotplt->size += 4;
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
      asection *sgot;
      bool dyn;
      int indx;

      int tls_type = csky_elf_hash_entry (h)->tls_type;
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local
	  && h->root.type == bfd_link_hash_undefweak
	  && ! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      sgot = htab->elf.sgot;
      h->got.offset = sgot->size;
      BFD_ASSERT (tls_type != GOT_UNKNOWN);
      if (tls_type == GOT_NORMAL)
	/* Non-TLS symbols need one GOT slot.  */
	sgot->size += 4;
      else
	{
	  if (tls_type & GOT_TLS_GD)
	    /* R_CKCORE_TLS_GD32 needs 2 consecutive GOT slots.  */
	    sgot->size += 8;
	  if (tls_type & GOT_TLS_IE)
	    /* R_CKCORE_TLS_IE32 needs one GOT slot.  */
	    sgot->size += 4;
	}
      dyn = htab->elf.dynamic_sections_created;
      indx = 0;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
	  && (! bfd_link_pic (info) || !SYMBOL_REFERENCES_LOCAL (info, h)))
	indx = h->dynindx;

      if (tls_type != GOT_NORMAL
	  && (bfd_link_pic (info) || indx != 0)
	  && ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	      || h->root.type != bfd_link_hash_undefweak))
	{
	  if (tls_type & GOT_TLS_IE)
	    htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
	  if (tls_type & GOT_TLS_GD)
	    htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
	  if ((tls_type & GOT_TLS_GD) && indx != 0)
	    htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
	}
      else if (((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		 && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
		|| h->root.type != bfd_link_hash_undefweak)
	       && (bfd_link_pic (info)
		   || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)
		   || h->plt.offset == (bfd_vma) -1))
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  eh = (struct csky_elf_link_hash_entry *) h;
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

      if (eh->jsri2bsr_refcount
	  && h->root.type == bfd_link_hash_defined
	  && h->dyn_relocs != NULL)
	h->dyn_relocs->count -= eh->jsri2bsr_refcount;

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
		   && !h->forced_local
		   && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic && !h->def_regular)
	      || (htab->elf.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_indirect
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1 && !h->forced_local
	      && h->root.type == bfd_link_hash_undefweak)
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
      asection *srelgot = htab->elf.srelgot;
      srelgot->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
csky_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info)
{
  struct csky_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;
  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    return false;

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (!bfd_link_pic (info) && !info->nointerp)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got_refcounts;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srelgot, *sgot;
      char *local_tls_type;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      sgot = htab->elf.sgot;
      srelgot = htab->elf.srelgot;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = elf_section_data (s)->local_dynrel;
	       p != NULL;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		/* Input section has been discarded, either because
		   it is a copy of a linkonce section or due to
		   linker script /DISCARD/, so we'll be discarding
		   the relocs too.  */
		;
	      else if (p->count != 0)
		{
		  srelgot->size += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got_refcounts = elf_local_got_refcounts (ibfd);
      if (!local_got_refcounts)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got_refcounts + locsymcount;
      local_tls_type = csky_elf_local_got_tls_type (ibfd);

      for (; local_got_refcounts < end_local_got;
	   ++local_got_refcounts, ++local_tls_type)
	{
	  if (*local_got_refcounts > 0)
	    {
	      /* GOT_TLS_GD and GOT_TLS_IE type for TLS, GOT_NORMAL type
		 for GOT.  If output file is shared library, we should output
		 GOT_TLS_GD type relocation in .rel.got.  */
	      *local_got_refcounts = sgot->size;
	      if (*local_tls_type & GOT_TLS_GD)
		/* TLS_GD relocs need an 8-byte structure in the GOT.  */
		sgot->size += 8;
	      if (*local_tls_type & GOT_TLS_IE)
		sgot->size += 4;
	      if (*local_tls_type == GOT_NORMAL)
		sgot->size += 4;
	      if (bfd_link_pic (info) || *local_tls_type == GOT_TLS_GD)
		srelgot->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got_refcounts = (bfd_vma) -1;
	}
    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate two GOT entries and one dynamic relocation (if necessary)
	 for R_CSKY_TLS_LDM32 relocations.  */
      htab->tls_ldm_got.offset = htab->elf.sgot->size;
      htab->elf.sgot->size += 8;
      if (bfd_link_pic (info))
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, csky_allocate_dynrelocs, info);

  /* Check for GOT overflow.  */
  if (check_got_overflow == 1
      && htab->elf.sgot->size + htab->elf.sgotplt->size > GOT_MAX_SIZE)
    {
      _bfd_error_handler (_("GOT table size out of range")); /*  */
      return false;
    }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      bool strip_section = true;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->elf.splt
	  || s == htab->elf.sgot
	  || s == htab->elf.sgotplt
	  || s == htab->elf.sdynrelro
	  || s == htab->elf.sreldynrelro)
	{
	  /* Strip this section if we don't need it;
	     see the comment below.  */
	  /* We'd like to strip these sections if they aren't needed, but if
	     we've exported dynamic symbols from them we must leave them.
	     It's too late to tell BFD to get rid of the symbols.  */

	  if (htab->elf.hplt != NULL)
	    strip_section = false;
	}
      else if (startswith (bfd_section_name (s), ".rel") )
	{
	  if (s->size != 0 )
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	/* It's not one of our sections, so don't allocate space.  */
	continue;

      /* Strip this section if we don't need it; see the
	 comment below.  */
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
	 but this way if it does, we get a R_CKCORE_NONE reloc instead
	 of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  if (htab->elf.dynamic_sections_created)
    htab->elf.dt_pltgot_required = htab->elf.sgot->size != 0;
  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
csky_elf_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  struct csky_elf_link_hash_table *htab;

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Sanity check to make sure no unexpected symbol reaches here.
     This matches the test in csky_elf_relocate_section handling
     of GOT/PLT entries.  */
  BFD_ASSERT (! (h->dynindx == -1
		 && !h->forced_local
		 && h->root.type != bfd_link_hash_undefweak
		 && bfd_link_pic (info)));

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      asection *plt, *relplt, *gotplt;

      plt = htab->elf.splt;
      relplt = htab->elf.srelplt;
      gotplt = htab->elf.sgotplt;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1
		  || ((h->forced_local || bfd_link_executable (info))
		      && h->def_regular));
      BFD_ASSERT (plt != NULL && gotplt != NULL && relplt != NULL);
      if (bfd_csky_abi (output_bfd) == CSKY_ABI_V2)
	plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
      else
	plt_index = h->plt.offset / PLT_ENTRY_SIZE_P - 1;
      got_offset = (plt_index + 3) * 4;

      /* Fill in the entry in the procedure linkage table.  */
      if (bfd_csky_abi (output_bfd) == CSKY_ABI_V2)
	{
	  csky_put_insn_32 (output_bfd, csky_elf_plt_entry_v2[0],
			    plt->contents + h->plt.offset);
	  csky_put_insn_32 (output_bfd,
			    (csky_elf_plt_entry_v2[1] | plt_index),
			    plt->contents + h->plt.offset + 4);
	  csky_put_insn_32 (output_bfd, csky_elf_plt_entry_v2[2],
			    plt->contents + h->plt.offset + 8);
	}
      else
	{
	  int i;
	  for (i = 0; i < 6; i++)
	    bfd_put_16 (output_bfd, csky_elf_plt_entry_v1[i],
			plt->contents + h->plt.offset + i * 2);
	  bfd_put_32 (output_bfd, plt_index,
		      plt->contents + h->plt.offset + i * 2);
	}

      /* Fill in the entry in the .rel.plt section.  */
      rel.r_offset = (htab->elf.sgotplt->output_section->vma
		      + htab->elf.sgotplt->output_offset
		      + got_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_CKCORE_JUMP_SLOT);
      rel.r_addend = (plt->output_section->vma
		      + plt->output_offset
		      + h->plt.offset);
      loc = (htab->elf.srelplt->contents
	     + plt_index * sizeof (Elf32_External_Rela));

      if (loc != NULL)
	bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
      if (! h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak, we do need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL. Leave the value if
	     there were any relocations where pointer equality matters
	     (this is a clue for the dynamic linker, to make function
	     pointer comparisons work between an application and shared
	     library).  */
	  if (!h->ref_regular_nonweak || !h->pointer_equality_needed)
	    sym->st_value = 0;
	}
    }

  /* Fill in the entry in the .got section.  */
  if (h->got.offset != (bfd_vma) -1
      && ((csky_elf_hash_entry (h)->tls_type & GOT_TLS_GD) == 0)
      && ((csky_elf_hash_entry (h)->tls_type & GOT_TLS_IE) == 0))
    {
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.
	 Set it up.  */
      BFD_ASSERT (htab->elf.sgot != NULL && htab->elf.srelgot != NULL);

      rel.r_offset = (htab->elf.sgot->output_section->vma
		      + htab->elf.sgot->output_offset
		      + (h->got.offset & ~(bfd_vma) 1));

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  BFD_ASSERT ((h->got.offset & 1) != 0);
	  rel.r_info = ELF32_R_INFO (0, R_CKCORE_RELATIVE);
	  rel.r_addend = (h->root.u.def.value
			  + h->root.u.def.section->output_offset
			  + h->root.u.def.section->output_section->vma);
	}
      else
	{
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgot->contents + h->got.offset);
	  rel.r_info = ELF32_R_INFO (h->dynindx, R_CKCORE_GLOB_DAT);
	  rel.r_addend = 0;
	}

      loc = htab->elf.srelgot->contents;
      loc += htab->elf.srelgot->reloc_count++ * sizeof (Elf32_External_Rela);

      if (loc != NULL)
	bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
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

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_CKCORE_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;
      BFD_ASSERT (s != NULL);
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
csky_elf_finish_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct csky_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;
  asection *got_sec;

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sdyn != NULL && htab->elf.sgot != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  bool size = false;
	  const char *name = NULL;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);
	  switch (dyn.d_tag)
	    {
	    default:
	      continue;
	    case DT_RELA:
	      name = ".rela.dyn";
	      size = false;
	      break;
	    case DT_RELASZ:
	      name = ".rela.dyn";
	      size = true;
	      break;
	    case DT_PLTRELSZ:
	      name = ".rela.plt";
	      size = true;
	      break;
	    case DT_PLTGOT:
	      dyn.d_un.d_ptr = htab->elf.sgot->output_section->vma;
	      break;
	    case DT_JMPREL:
	      dyn.d_un.d_ptr = htab->elf.srelplt->output_section->vma
			       + htab->elf.srelplt->output_offset;
	      break;
	    }

	  if (name != NULL)
	    {
	      asection *s = bfd_get_section_by_name (output_bfd, name);

	      if (s == NULL)
		dyn.d_un.d_val = 0;
	      else if (!size)
		dyn.d_un.d_ptr = s->vma;
	      else
		dyn.d_un.d_val = s->size;
	    }
	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}
    }

  /* Fill in the first three entries in the global offset table.  */
  if (htab->elf.sgotplt)
    got_sec = htab->elf.sgotplt;
  else
    got_sec = htab->elf.sgot;
  if (got_sec != NULL)
    {
      if (got_sec->size > 0)
	{
	  bfd_put_32 (output_bfd,
		      (sdyn == NULL ? (bfd_vma) 0
		       : sdyn->output_section->vma + sdyn->output_offset),
		      got_sec->contents);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, got_sec->contents + 4);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, got_sec->contents + 8);
	}
      elf_section_data (got_sec->output_section)->this_hdr.sh_entsize = 4;
    }
  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
csky_elf_copy_indirect_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *dir,
			       struct elf_link_hash_entry *ind)
{
  struct csky_elf_link_hash_entry *edir, *eind;

  edir = (struct csky_elf_link_hash_entry *) dir;
  eind = (struct csky_elf_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }
  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
csky_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_CKCORE_RELATIVE:
      return reloc_class_relative;
    case R_CKCORE_JUMP_SLOT:
      return reloc_class_plt;
    case R_CKCORE_COPY:
      return reloc_class_copy;
    case R_CKCORE_IRELATIVE:
      return reloc_class_ifunc;
    default:
      return reloc_class_normal;
    }
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
csky_elf_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_CKCORE_GNU_VTINHERIT:
	case R_CKCORE_GNU_VTENTRY:
	  return NULL;
	}
    }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Match symbol names created by tc-csky.c:make_mapping_symbol.  */

static bool
is_mapping_symbol_name (const char *name)
{
  return (name && name[0] == '$'
	  && (name[1] == 't' || name[1] == 'd')
	  && name[2] == 0);
}

/* Treat mapping symbols as special target symbols.  */

static bool
csky_elf_is_target_special_symbol (bfd *abfd ATTRIBUTE_UNUSED, asymbol *sym)
{
  return is_mapping_symbol_name (sym->name);
}

/* Exclude mapping symbols from being treated as function symbols by
   objdump and nm.  */

static bfd_size_type
csky_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			     bfd_vma *code_off)
{
  if ((sym->flags & BSF_LOCAL) != 0
      && is_mapping_symbol_name (sym->name))
    return 0;

  return _bfd_elf_maybe_function_sym (sym, sec, code_off);
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
csky_elf_check_relocs (bfd * abfd,
		       struct bfd_link_info * info,
		       asection * sec,
		       const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr * symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela * rel;
  const Elf_Internal_Rela * rel_end;
  struct csky_elf_link_hash_table *htab;
  asection *sreloc;

  /* if output type is relocatable, return.  */
  if (bfd_link_relocatable (info))
    return true;

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  sreloc = NULL;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      Elf_Internal_Sym *isym;
      int r_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
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
	case R_CKCORE_PCREL_IMM26BY2:
	case R_CKCORE_PCREL_IMM11BY2:
	case R_CKCORE_PCREL_JSR_IMM11BY2:
	case R_CKCORE_PCREL_JSR_IMM26BY2:
	  /* If the symbol is '*UND*', means this reloc is used for
	   * callgraph, don't need to leave to shared object. */
	  if (r_symndx == 0)
	    break;
	  /* Else fall through.  */
	case R_CKCORE_ADDR32:
	case R_CKCORE_ADDR_HI16:
	case R_CKCORE_ADDR_LO16:
	  if (h != NULL
	      && bfd_link_executable (info)
	      && r_type == R_CKCORE_ADDR32
	      && h->type == STT_OBJECT
	      && (sec->flags & SEC_ALLOC) != 0
	      && (sec->flags & SEC_READONLY))
	    /* If this reloc is in a read-only section, we might
	       need a copy reloc.  We can't check reliably at this
	       stage whether the section is read-only, as input
	       sections have not yet been mapped to output sections.
	       Tentatively set the flag for now, and correct in
	       adjust_dynamic_symbol.  */
	    h->non_got_ref = 1;

	  /* If we are creating a shared library or relocatable executable,
	     and this is a reloc against a global symbol, then we need to
	     copy the reloc into the shared library. However, if we are
	     linking with -Bsymbolic, we do not need to copy a reloc
	     against a global symbol which is defined in an object we are
	     including in the link (i.e., DEF_REGULAR is set).  At
	     this point we have not seen all the input files, so it is
	     possible that DEF_REGULAR is not set now but will be set
	     later (it is never cleared). We account for that possibility
	     below by storing information in the relocs_copied field of
	     the hash table entry.  */
	  if ((bfd_link_pic (info) && (sec->flags & SEC_ALLOC) != 0)
	      || (!bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
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
		    (sec, htab->elf.dynobj, 2, abfd, true);

		  if (sreloc == NULL)
		    return false;
		}

	      if (h == NULL && !use_branch_stub
		  && ((ELF32_R_TYPE (rel->r_info)
		       == R_CKCORE_PCREL_IMM26BY2)
		      || (ELF32_R_TYPE (rel->r_info)
			  == R_CKCORE_PCREL_IMM11BY2)))
		break;

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  struct csky_elf_link_hash_entry *eh;
		  eh = (struct  csky_elf_link_hash_entry *)h;
		  if ((ELF32_R_TYPE (rel->r_info)
		       == R_CKCORE_PCREL_JSR_IMM26BY2)
		      || (ELF32_R_TYPE (rel->r_info)
			  == R_CKCORE_PCREL_JSR_IMM11BY2))
		    eh->jsri2bsr_refcount += 1;
		  head = &h->dyn_relocs;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */
		  void **vpp;
		  asection *s;
		  Elf_Internal_Sym *loc_isym;

		  loc_isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
						    abfd, r_symndx);
		  if (loc_isym == NULL)
		    return false;
		  s = bfd_section_from_elf_index (abfd, loc_isym->st_shndx);
		  if (s == NULL)
		    s = sec;
		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **)vpp;
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

	      if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_PCREL_IMM26BY2
		  || ELF32_R_TYPE (rel->r_info) == R_CKCORE_PCREL_IMM11BY2)
		p->pc_count += 1;
	      p->count += 1;
	    }
	  break;

	case R_CKCORE_PLT_IMM18BY4:
	case R_CKCORE_PLT32:
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
	  if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_PLT_IMM18BY4)
	    check_got_overflow = 1;

	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  h->got.refcount += 1;
	  ((struct  csky_elf_link_hash_entry *)h)->plt_refcount += 1;
	  break;

	case R_CKCORE_GOT12:
	case R_CKCORE_PLT12:
	case R_CKCORE_GOT32:
	case R_CKCORE_GOT_HI16:
	case R_CKCORE_GOT_LO16:
	case R_CKCORE_PLT_HI16:
	case R_CKCORE_PLT_LO16:
	case R_CKCORE_GOT_IMM18BY4:
	case R_CKCORE_TLS_IE32:
	case R_CKCORE_TLS_GD32:
	  {
	    int tls_type, old_tls_type;

	    if (h != NULL
		&& bfd_link_executable (info)
		&& r_type == R_CKCORE_GOT_IMM18BY4
		&& (sec->flags & SEC_ALLOC) != 0
		&& (sec->flags & SEC_READONLY))
	      /* If this reloc is in a read-only section, we might
		 need a copy reloc.  We can't check reliably at this
		 stage whether the section is read-only, as input
		 sections have not yet been mapped to output sections.
		 Tentatively set the flag for now, and correct in
		 adjust_dynamic_symbol.  */
	      h->non_got_ref = 1;

	    switch (ELF32_R_TYPE (rel->r_info))
	      {
	      case R_CKCORE_TLS_IE32:
		tls_type = GOT_TLS_IE;
		break;
	      case R_CKCORE_TLS_GD32:
		tls_type = GOT_TLS_GD;
		break;
	      default:
		tls_type = GOT_NORMAL;
		break;
	      }
	    if (h != NULL)
	      {
		if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_GOT_IMM18BY4)
		  check_got_overflow = 1;
		h->got.refcount += 1;
		old_tls_type = csky_elf_hash_entry (h)->tls_type;
	      }
	    else
	      {
		bfd_signed_vma *local_got_refcounts;

		/* This is a global offset table entry for a local symbol.  */
		/* we can write a new function named
		   elf32_csky_allocate_local_sym_info() to replace
		   following code.  */
		local_got_refcounts = elf_local_got_refcounts (abfd);
		if (local_got_refcounts == NULL)
		  {
		    bfd_size_type size;

		    size = symtab_hdr->sh_info;
		    size *= (sizeof (bfd_signed_vma) + sizeof (char));
		    local_got_refcounts = ((bfd_signed_vma *)
					   bfd_zalloc (abfd, size));
		    if (local_got_refcounts == NULL)
		      return false;
		    elf_local_got_refcounts (abfd) = local_got_refcounts;
		    csky_elf_local_got_tls_type (abfd)
		      = (char *) (local_got_refcounts + symtab_hdr->sh_info);
		  }
		local_got_refcounts[r_symndx] += 1;
		old_tls_type = csky_elf_local_got_tls_type (abfd)[r_symndx];
	      }

	    /* We will already have issued an error message if there is a
	       TLS / non-TLS mismatch, based on the symbol type.  We don't
	       support any linker relaxations.  So just combine any TLS
	       types needed.  */
	    if (old_tls_type != GOT_UNKNOWN && old_tls_type != GOT_NORMAL
		&& tls_type != GOT_NORMAL)
	      tls_type |= old_tls_type;

	    if (old_tls_type != tls_type)
	      {
		if (h != NULL)
		  csky_elf_hash_entry (h)->tls_type = tls_type;
		else
		  csky_elf_local_got_tls_type (abfd)[r_symndx] = tls_type;
	      }
	  }
	  /* Fall through.  */

	case R_CKCORE_TLS_LDM32:
	  if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_TLS_LDM32)
	    htab->tls_ldm_got.refcount++;
	  /* Fall through.  */

	case R_CKCORE_GOTOFF:
	case R_CKCORE_GOTPC:
	case R_CKCORE_GOTOFF_HI16:
	case R_CKCORE_GOTOFF_LO16:
	case R_CKCORE_GOTPC_HI16:
	case R_CKCORE_GOTPC_LO16:
	case R_CKCORE_GOTOFF_IMM18:
	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_CKCORE_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_CKCORE_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;
	}
    }

  return true;
}

static const struct bfd_elf_special_section csky_elf_special_sections[]=
{
  { STRING_COMMA_LEN (".ctors"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".dtors"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { NULL,                     0,  0, 0,            0 }
};

/* Function to keep CSKY specific flags in the ELF header.  */

static bool
csky_elf_set_private_flags (bfd * abfd, flagword flags)
{
  BFD_ASSERT (! elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

static csky_arch_for_merge *
csky_find_arch_with_eflag (const unsigned long arch_eflag)
{
  csky_arch_for_merge *csky_arch = NULL;

  for (csky_arch = csky_archs; csky_arch->name != NULL; csky_arch++)
    if (csky_arch->arch_eflag == arch_eflag)
      break;
  if (csky_arch == NULL)
    {
      _bfd_error_handler (_("warning: unrecognized arch eflag '%#lx'"),
			   arch_eflag);
      bfd_set_error (bfd_error_wrong_format);
    }
  return csky_arch;
}

static csky_arch_for_merge *
csky_find_arch_with_name (const char *name)
{
  csky_arch_for_merge *csky_arch = NULL;
  const char *msg;

  if (name == NULL)
    return NULL;

  for (csky_arch = csky_archs; csky_arch->name != NULL; csky_arch++)
    {
      if (strncmp (csky_arch->name, name, strlen (csky_arch->name)) == 0)
	break;
    }
  if (csky_arch == NULL)
    {
      msg = _("warning: unrecognised arch name '%#x'");
      (*_bfd_error_handler) (msg, name);
      bfd_set_error (bfd_error_wrong_format);
    }
  return csky_arch;
}

static bool
elf32_csky_merge_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  csky_arch_for_merge *old_arch = NULL;
  csky_arch_for_merge *new_arch = NULL;
  int i;
  bool result = true;
  const char *msg = NULL;

  const char *sec_name = get_elf_backend_data (ibfd)->obj_attrs_section;

  /* Skip the linker stubs file.  This preserves previous behavior
     of accepting unknown attributes in the first input file - but
     is that a bug?  */
  if (ibfd->flags & BFD_LINKER_CREATED)
    return true;

  /* Skip any input that hasn't attribute section.
     This enables to link object files without attribute section with
     any others.  */
  if (bfd_get_section_by_name (ibfd, sec_name) == NULL)
    {
      return true;
    }

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      out_attr = elf_known_obj_attributes_proc (obfd);

      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      out_attr[0].i = 1;
    }

  in_attr = elf_known_obj_attributes_proc (ibfd);
  out_attr = elf_known_obj_attributes_proc (obfd);

  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    {
      /* Merge this attribute with existing attributes.  */
      switch (i)
        {
	case Tag_CSKY_CPU_NAME:
	case Tag_CSKY_ARCH_NAME:
	  /* Do arch merge.  */
	  new_arch = csky_find_arch_with_name (in_attr[Tag_CSKY_ARCH_NAME].s);
	  old_arch = csky_find_arch_with_name (out_attr[Tag_CSKY_ARCH_NAME].s);

	  if (new_arch != NULL && old_arch != NULL)
	    {
	      if (new_arch->class != old_arch->class)
		{
		  msg = _("%pB: machine flag conflict with target");
		  (*_bfd_error_handler) (msg, ibfd);
		  bfd_set_error (bfd_error_wrong_format);
		  return false;
		}
	      else if (new_arch->class_level != old_arch->class_level)
		{
		  csky_arch_for_merge *newest_arch =
		    ((new_arch->class_level > old_arch->class_level) ?
		  new_arch : old_arch);

		  if (new_arch->do_warning || old_arch->do_warning)
		    {
		      msg = _("warning: file %pB's arch flag %s conflict "
			      "with target %s,set target arch flag to %s");
		      (*_bfd_error_handler) (msg, ibfd,  new_arch->name,
					     old_arch->name,
					     (newest_arch->name));
		      bfd_set_error (bfd_error_wrong_format);
                    }

		  if (out_attr[Tag_CSKY_ARCH_NAME].s != NULL)
		    bfd_release (obfd, out_attr[Tag_CSKY_ARCH_NAME].s);

		  out_attr[Tag_CSKY_ARCH_NAME].s =
		    _bfd_elf_attr_strdup (obfd, newest_arch->name);
		}
	    }

	  break;

	case Tag_CSKY_ISA_FLAGS:
	case Tag_CSKY_ISA_EXT_FLAGS:
	  /* Do ISA merge.  */
	  break;

	case Tag_CSKY_VDSP_VERSION:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (out_attr[i].i != in_attr[i].i)
	    {
	      _bfd_error_handler
		(_("Error: %pB and %pB has different VDSP version"), ibfd, obfd);
	      result = false;
	    }
	  break;

	case Tag_CSKY_FPU_VERSION:
	  if (out_attr[i].i <= in_attr[i].i
	      && out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  break;

	case Tag_CSKY_DSP_VERSION:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (out_attr[i].i != in_attr[i].i)
	    {
	      _bfd_error_handler
		(_("Error: %pB and %pB has different DSP version"), ibfd, obfd);
	      result = false;
	    }
	  break;

	case Tag_CSKY_FPU_ABI:
	  if (out_attr[i].i != in_attr[i].i
	      && (out_attr[i].i == 0
		  || (out_attr[i].i == VAL_CSKY_FPU_ABI_SOFT
		      && in_attr[i].i == VAL_CSKY_FPU_ABI_SOFTFP)))
	    {
	      out_attr[i].i = in_attr[i].i;
	    }
	  else if (out_attr[i].i == VAL_CSKY_FPU_ABI_HARD
		   && (out_attr[i].i != in_attr[i].i
		       && in_attr[i].i != 0))
	    {
	      _bfd_error_handler
	       (_("Error: %pB and %pB has different FPU ABI"), ibfd, obfd);
	       result = false;
	    }
	  break;

	default:
	  result =
	    result && _bfd_elf_merge_unknown_attribute_low (ibfd, obfd, i);
	  break;
	}

      /* If out_attr was copied from in_attr then it won't have a type yet.  */
      if (in_attr[i].type && !out_attr[i].type)
	out_attr[i].type = in_attr[i].type;
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  if (!_bfd_elf_merge_object_attributes (ibfd, info))
    return false;

  /* Check for any attributes not known on CSKY.  */
  result &= _bfd_elf_merge_unknown_attribute_list (ibfd, obfd);

  return result;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
csky_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags;
  flagword new_flags;
  csky_arch_for_merge *old_arch = NULL;
  csky_arch_for_merge *new_arch = NULL;
  flagword newest_flag = 0;
  const char *sec_name;
  obj_attribute *out_attr;

  /* Check if we have the same endianness.  */
  if (! _bfd_generic_verify_endian_match (ibfd, info))
    return false;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  /* Merge ".csky.attribute" section.  */
  if (!elf32_csky_merge_attributes (ibfd, info))
    return false;

  if (! elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = true;
    }

  /* Try to merge e_flag.  */
  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;
  out_attr = elf_known_obj_attributes_proc (obfd);

  /* The flags like "e , f ,g ..." , we take collection.  */
  newest_flag = old_flags | new_flags;

  sec_name = get_elf_backend_data (ibfd)->obj_attrs_section;

  if (bfd_get_section_by_name (ibfd, sec_name) == NULL
      || ((new_flags & (CSKY_ARCH_MASK | CSKY_ABI_MASK)) !=
	  (old_flags & (CSKY_ARCH_MASK | CSKY_ABI_MASK))))
    {
      /* Input BFDs have no ".csky.attribute" section.  */
      new_arch = csky_find_arch_with_eflag (new_flags & CSKY_ARCH_MASK);
      old_arch = csky_find_arch_with_name (out_attr[Tag_CSKY_ARCH_NAME].s);

      if (new_arch != NULL && old_arch != NULL)
	{
	  if (new_arch->class != old_arch->class)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: machine flag conflict with target"), ibfd);
	      bfd_set_error (bfd_error_wrong_format);
	      return false;
	    }
	  else if (new_arch->class_level != old_arch->class_level)
	    {
	      csky_arch_for_merge *newest_arch =
		(new_arch->class_level > old_arch->class_level
		 ? new_arch : old_arch);

	      if (new_arch->do_warning || old_arch->do_warning)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("warning: file %pB's arch flag %s conflicts with "
		       "target ck%s, using %s"),
		     ibfd, new_arch->name, old_arch->name,
		     newest_arch->name);
		  bfd_set_error (bfd_error_wrong_format);
		}

	      if (out_attr[Tag_CSKY_ARCH_NAME].s != NULL)
		bfd_release (obfd, out_attr[Tag_CSKY_ARCH_NAME].s);

	      out_attr[Tag_CSKY_ARCH_NAME].s =
		_bfd_elf_attr_strdup (obfd, newest_arch->name);
	    }
	}
      else
	{
	  if (new_arch && new_arch->name != NULL)
	    out_attr[Tag_CSKY_ARCH_NAME].s =
	  _bfd_elf_attr_strdup (obfd, new_arch->name);
	}
    }

  elf_elfheader (obfd)->e_flags = newest_flag;

  return true;
}

/* Ignore the discarded relocs in special sections in link time.  */

static bool
csky_elf_ignore_discarded_relocs (asection *sec)
{
  if (strcmp (sec->name, ".csky_stack_size") == 0)
    return true;
  return false;
}

/* .csky_stack_size are not referenced directly.  This pass marks all of
   them as required.  */

static bool
elf32_csky_gc_mark_extra_sections (struct bfd_link_info *info,
				   elf_gc_mark_hook_fn gc_mark_hook ATTRIBUTE_UNUSED)
{
  bfd *sub;

  _bfd_elf_gc_mark_extra_sections (info, gc_mark_hook);

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *o;

      for (o = sub->sections; o != NULL; o = o->next)
	if (strcmp (o->name, ".csky_stack_size") == 0)
	  o->gc_mark = 1;
    }

  return true;
}

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */

void
elf32_csky_next_input_section (struct bfd_link_info *info,
			       asection *isec)
{
  struct csky_elf_link_hash_table *htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return;
  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;

      if (*list != bfd_abs_section_ptr)
	{
	  /* Steal the link_sec pointer for our list.  */
#define PREV_SEC(sec) (htab->stub_group[(sec)->id].link_sec)
	  /* This happens to make the list in reverse order,
	     which we reverse later in group_sections.  */
	  PREV_SEC (isec) = *list;
	  *list = isec;
	}
    }
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the end of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static void
group_sections (struct csky_elf_link_hash_table *htab,
		bfd_size_type stub_group_size,
		bool stubs_always_after_branch)
{
  asection **list = htab->input_list;

  do
    {
      asection *tail = *list;
      asection *head;

      if (tail == bfd_abs_section_ptr)
	continue;

      /* Reverse the list: we must avoid placing stubs at the
	 beginning of the section because the beginning of the text
	 section may be required for an interrupt vector in bare metal
	 code.  */
#define NEXT_SEC PREV_SEC
      head = NULL;
      while (tail != NULL)
	{
	  /* Pop from tail.  */
	  asection *item = tail;
	  tail = PREV_SEC (item);

	  /* Push on head.  */
	  NEXT_SEC (item) = head;
	  head = item;
	}

      while (head != NULL)
	{
	  asection *curr;
	  asection *next;
	  bfd_vma stub_group_start = head->output_offset;
	  bfd_vma end_of_next;

	  curr = head;
	  while (NEXT_SEC (curr) != NULL)
	    {
	      next = NEXT_SEC (curr);
	      end_of_next = next->output_offset + next->size;
	      if (end_of_next - stub_group_start >= stub_group_size)
		/* End of NEXT is too far from start, so stop.  */
		break;
	      curr = next;
	    }

	  /* OK, the size from the start to the start of CURR is less
	   * than stub_group_size and thus can be handled by one stub
	   * section.  (Or the head section is itself larger than
	   * stub_group_size, in which case we may be toast.)
	   * We should really be keeping track of the total size of
	   * stubs added here, as stubs contribute to the final output
	   * section size.  */
	  do
	    {
	      next = NEXT_SEC (head);
	      /* Set up this stub group.  */
	      htab->stub_group[head->id].link_sec = curr;
	    }
	  while (head != curr && (head = next) != NULL);

	  /* But wait, there's more!  Input sections up to stub_group_size
	   * bytes after the stub section can be handled by it too.  */
	  if (!stubs_always_after_branch)
	    {
	      stub_group_start = curr->output_offset + curr->size;

	      while (next != NULL)
		{
		  end_of_next = next->output_offset + next->size;
		  if (end_of_next - stub_group_start >= stub_group_size)
		    /* End of NEXT is too far from stubs, so stop.  */
		    break;
		  /* Add NEXT to the stub group.  */
		  head = next;
		  next = NEXT_SEC (head);
		  htab->stub_group[head->id].link_sec = curr;
		}
	    }
	  head = next;
	}
    }
  while (list++ != htab->input_list + htab->top_index);

  free (htab->input_list);
#undef PREV_SEC
#undef NEXT_SEC
}

/* If the symbol referenced by bsr is defined in shared object file,
   or it is a weak symbol and we aim to create shared object file,
   we must create a stub for this bsr.  */

static bool
sym_must_create_stub (struct elf_link_hash_entry *h,
		      struct bfd_link_info *info)
{
  if (h != NULL
      && ((h->def_dynamic && !h->def_regular)
	  || (bfd_link_pic (info) && h->root.type == bfd_link_hash_defweak)))
    return true;
  else
    return false;
}

/* Calculate the template, template size and instruction size for a stub.
   Return value is the instruction size.  */

static unsigned int
find_stub_size_and_template (enum elf32_csky_stub_type stub_type,
			     const insn_sequence **stub_template,
			     int *stub_template_size)
{
  const insn_sequence *template_sequence = NULL;
  int template_size = 0;
  int i;
  unsigned int size;

  template_sequence = stub_definitions[stub_type].template_sequence;
  template_size = stub_definitions[stub_type].template_size;

  size = 0;
  for (i = 0; i < template_size; i++)
    {
      switch (template_sequence[i].type)
      {
      case INSN16:
	size += 2;
	break;

      case INSN32:
      case DATA_TYPE:
	size += 4;
	break;

      default:
	BFD_FAIL ();
	return false;
      }
    }

  if (stub_template)
    *stub_template = template_sequence;
  if (stub_template_size)
    *stub_template_size = template_size;

  return size;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bool
csky_size_one_stub (struct bfd_hash_entry *gen_entry,
		    void * in_arg ATTRIBUTE_UNUSED)
{
  struct elf32_csky_stub_hash_entry *stub_entry;
  const insn_sequence *template_sequence = NULL;
  int template_size = 0;
  int size = 0;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_csky_stub_hash_entry *) gen_entry;

  BFD_ASSERT (stub_entry->stub_type > csky_stub_none
	      && stub_entry->stub_type < ARRAY_SIZE (stub_definitions));
  size = find_stub_size_and_template (stub_entry->stub_type,
				      &template_sequence, &template_size);
  stub_entry->stub_size = size;
  stub_entry->stub_template = template_sequence;
  stub_entry->stub_template_size = template_size;

  size = (size + 7) & ~7;
  stub_entry->stub_sec->size += size;
  return true;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf32_csky_stub_hash_entry *
elf32_csky_add_stub (const char *stub_name,
		     asection *section,
		     struct csky_elf_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;
  struct elf32_csky_stub_hash_entry *stub_entry;

  stub_sec = elf32_csky_create_or_find_stub_sec (&link_sec, section, htab);
  if (stub_sec == NULL)
    return NULL;

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = csky_stub_hash_lookup (&htab->stub_hash_table, stub_name,
				      true, false);
  if (stub_entry == NULL)
    {
      _bfd_error_handler (_("%pB: cannot create stub entry %s"),
			  section->owner, stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = 0;
  stub_entry->id_sec = link_sec;

  return stub_entry;
}

/* Determine and set the size of the stub section for a final link.
   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bsr"
   instruction.  */

bool
elf32_csky_size_stubs (bfd *output_bfd,
		       bfd *stub_bfd,
		       struct bfd_link_info *info,
		       bfd_signed_vma group_size,
		       asection *(*add_stub_section) (const char*, asection*),
		       void (*layout_sections_again) (void))
{
  bfd_size_type stub_group_size;
  bool stubs_always_after_branch;
  struct csky_elf_link_hash_table *htab = csky_elf_hash_table (info);

  if (htab == NULL)
    return false;

  /* Propagate mach to stub bfd, because it may not have been
     finalized when we created stub_bfd.  */
  bfd_set_arch_mach (stub_bfd, bfd_get_arch (output_bfd),
		     bfd_get_mach (output_bfd));

  /* Stash our params away.  */
  htab->stub_bfd = stub_bfd;
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;
  stubs_always_after_branch = group_size < 0;

  if (group_size < 0)
    stub_group_size = -group_size;
  else
    stub_group_size = group_size;

  if (stub_group_size == 1)
    /* The 'bsr' range in abiv2 is +-64MB has to be used as the
       default maximum size.
       This value is 128K less than that, which allows for 131072
       byte stubs. If we exceed that, then we will fail to link.
       The user will have to relink with an explicit group size
       option.  */
    stub_group_size = 66977792;

  group_sections (htab, stub_group_size, stubs_always_after_branch);

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      asection *stub_sec;
      bool stub_changed = false;

      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms = NULL;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	      /* If there aren't any relocs, then there's nothing more
	       * to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0
		  || (section->flags & SEC_CODE) == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs = _bfd_elf_link_read_relocs (input_bfd,
							   section,
							   NULL, NULL,
							   info->keep_memory);

	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  unsigned int r_type, r_indx;
		  enum elf32_csky_stub_type stub_type;
		  struct elf32_csky_stub_hash_entry *stub_entry;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct csky_elf_link_hash_entry *hash;
		  const char *sym_name;
		  char *stub_name;
		  const asection *id_sec;
		  unsigned char st_type;

		  r_type = ELF32_R_TYPE (irela->r_info);
		  r_indx = ELF32_R_SYM (irela->r_info);
		  if (r_type >= (unsigned int) R_CKCORE_MAX)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }

		  /* Only look for stubs on branch instructions.  */
		  if (r_type != (unsigned int) R_CKCORE_PCREL_IMM26BY2)
		    continue;
		  /* Now determine the call target, its name, value,
		     section.  */
		  sym_sec = NULL;
		  sym_value = 0;
		  destination = 0;
		  hash = NULL;
		  sym_name = NULL;
		  if (r_indx < symtab_hdr->sh_info)
		    {
		      /* It's a local symbol.  */
		      Elf_Internal_Sym *sym;
		      Elf_Internal_Shdr *hdr;
		      if (local_syms == NULL)
			local_syms =
			  (Elf_Internal_Sym *) symtab_hdr->contents;
		      if (local_syms == NULL)
			{
			  local_syms =
			    bfd_elf_get_elf_syms (input_bfd,
						  symtab_hdr,
						  symtab_hdr->sh_info,
						  0, NULL, NULL, NULL);
			  if (local_syms == NULL)
			    goto error_ret_free_internal;
			}
		      sym = local_syms + r_indx;
		      hdr = elf_elfsections (input_bfd)[sym->st_shndx];
		      sym_sec = hdr->bfd_section;
		      if (!sym_sec)
			/* This is an undefined symbol.  It can never
			   be resolved.  */
			continue;
		      if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
			sym_value = sym->st_value;
		      destination = (sym_value + irela->r_addend
				     + sym_sec->output_offset
				     + sym_sec->output_section->vma);
		      st_type = ELF_ST_TYPE (sym->st_info);
		      sym_name =
			bfd_elf_string_from_elf_section (input_bfd,
							 symtab_hdr->sh_link,
							 sym->st_name);
		    }
		  else
		    {
		      /* It's an external symbol.  */
		      int e_indx;
		      e_indx = r_indx - symtab_hdr->sh_info;
		      hash = ((struct csky_elf_link_hash_entry *)
			      elf_sym_hashes (input_bfd)[e_indx]);

		      while (hash->elf.root.type == bfd_link_hash_indirect
			     || hash->elf.root.type == bfd_link_hash_warning)
			hash = ((struct csky_elf_link_hash_entry *)
				hash->elf.root.u.i.link);
		      if (hash->elf.root.type == bfd_link_hash_defined
			  || hash->elf.root.type == bfd_link_hash_defweak)
			{
			  sym_sec = hash->elf.root.u.def.section;
			  sym_value = hash->elf.root.u.def.value;

			  struct csky_elf_link_hash_table *globals =
			    csky_elf_hash_table (info);
			  /* FIXME For a destination in a shared library.  */
			  if (globals->elf.splt != NULL && hash != NULL
			      && hash->elf.plt.offset != (bfd_vma) -1)
			    continue;
			  else if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			}
		      else if (hash->elf.root.type == bfd_link_hash_undefined
			       || (hash->elf.root.type
				   == bfd_link_hash_undefweak))
			/* FIXME For a destination in a shared library.  */
			continue;
		      else
			{
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret_free_internal;
			}
		      st_type = ELF_ST_TYPE (hash->elf.type);
		      sym_name = hash->elf.root.root.string;
		    }
		  do
		    {
		      /* Determine what (if any) linker stub is needed.  */
		      stub_type = csky_type_of_stub (info, section, irela,
						     st_type, hash,
						     destination, sym_sec,
						     input_bfd, sym_name);
		      if (stub_type == csky_stub_none)
			break;

		      /* Support for grouping stub sections.  */
		      id_sec = htab->stub_group[section->id].link_sec;

		      /* Get the name of this stub.  */
		      stub_name = elf32_csky_stub_name (id_sec, sym_sec, hash,
							irela);
		      if (!stub_name)
			goto error_ret_free_internal;
		      /* We've either created a stub for this reloc already,
			 or we are about to.  */
		      stub_entry
			= csky_stub_hash_lookup	(&htab->stub_hash_table,
						 stub_name,
						 false, false);
		      if (stub_entry != NULL)
			{
			  /* The proper stub has already been created.  */
			  free (stub_name);
			  stub_entry->target_value = sym_value;
			  break;
			}
		      stub_entry = elf32_csky_add_stub (stub_name, section,
							htab);
		      if (stub_entry == NULL)
			{
			  free (stub_name);
			  goto error_ret_free_internal;
			}
		      stub_entry->target_value = sym_value;
		      stub_entry->target_section = sym_sec;
		      stub_entry->stub_type = stub_type;
		      stub_entry->h = hash;
		      stub_entry->st_type = st_type;

		      if (sym_name == NULL)
			sym_name = "unnamed";
		      stub_entry->output_name =
			bfd_alloc (htab->stub_bfd,
				   (sizeof (STUB_ENTRY_NAME)
				    + strlen (sym_name)));
		      if (stub_entry->output_name == NULL)
			{
			  free (stub_name);
			  goto error_ret_free_internal;
			}
		      sprintf (stub_entry->output_name, STUB_ENTRY_NAME,
			       sym_name);
		      stub_changed = true;
		    }
		  while (0);
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
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;
	  stub_sec->size = 0;
	}
      bfd_hash_traverse (&htab->stub_hash_table, csky_size_one_stub, htab);
      /* Ask the linker to do its stuff.  */
      (*htab->layout_sections_again) ();
    }

  return true;
 error_ret_free_local:
  return false;
}

static bool
csky_build_one_stub (struct bfd_hash_entry *gen_entry,
		     void * in_arg)
{
#define MAXRELOCS 2
  struct elf32_csky_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_byte *loc;
  bfd_vma sym_value;
  int template_size;
  int size;
  const insn_sequence *template_sequence;
  int i;
  struct csky_elf_link_hash_table * globals;
  int stub_reloc_idx[MAXRELOCS] = {-1, -1};
  int stub_reloc_offset[MAXRELOCS] = {0, 0};
  int nrelocs = 0;
  struct elf_link_hash_entry *h = NULL;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf32_csky_stub_hash_entry *)gen_entry;
  info = (struct bfd_link_info *) in_arg;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (stub_entry->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    stub_entry->target_section);

  globals = csky_elf_hash_table (info);
  if (globals == NULL)
    return false;
  stub_sec = stub_entry->stub_sec;

  /* Make a note of the offset within the stubs for this entry.  */
  stub_entry->stub_offset = stub_sec->size;
  loc = stub_sec->contents + stub_entry->stub_offset;

  stub_bfd = stub_sec->owner;

  /* This is the address of the stub destination.  */
  h = &stub_entry->h->elf;
  if (sym_must_create_stub (h, info)
      && !(bfd_link_pic (info)
	   && h->root.type == bfd_link_hash_defweak
	   && h->def_regular
	   && !h->def_dynamic))
    sym_value = 0;
  else
    sym_value = (stub_entry->target_value
		 + stub_entry->target_section->output_offset
		 + stub_entry->target_section->output_section->vma);

  template_sequence = stub_entry->stub_template;
  template_size = stub_entry->stub_template_size;

  size = 0;
  for (i = 0; i < template_size; i++)
    switch (template_sequence[i].type)
      {
      case INSN16:
	bfd_put_16 (stub_bfd, (bfd_vma) template_sequence[i].data,
		    loc + size);
	size += 2;
	break;
      case INSN32:
	csky_put_insn_32 (stub_bfd, (bfd_vma) template_sequence[i].data,
			  loc + size);
	size += 4;
	break;
      case DATA_TYPE:
	bfd_put_32 (stub_bfd, (bfd_vma) template_sequence[i].data,
		    loc + size);
	stub_reloc_idx[nrelocs] = i;
	stub_reloc_offset[nrelocs++] = size;
	size += 4;
	break;
      default:
	BFD_FAIL ();
	return false;
      }
  stub_sec->size += size;

  /* Stub size has already been computed in csky_size_one_stub. Check
     consistency.  */
  BFD_ASSERT (size == stub_entry->stub_size);

  /* Assume there is at least one and at most MAXRELOCS entries to relocate
     in each stub.  */
  BFD_ASSERT (nrelocs != 0 && nrelocs <= MAXRELOCS);

  for (i = 0; i < nrelocs; i++)
    {
      if (sym_must_create_stub (h, info))
	{
	  Elf_Internal_Rela outrel;
	  asection * sreloc = globals->elf.srelgot;

	  outrel.r_offset = stub_entry->stub_offset + stub_reloc_offset[i];
	  outrel.r_info =
	    ELF32_R_INFO (h->dynindx,
			  template_sequence[stub_reloc_idx[i]].r_type);
	  outrel.r_addend = template_sequence[stub_reloc_idx[i]].reloc_addend;

	  loc = sreloc->contents;
	  loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);

	  if (loc != NULL)
	    bfd_elf32_swap_reloca_out (info->output_bfd, &outrel, loc);
	}
      _bfd_final_link_relocate (elf32_csky_howto_from_type
				  (template_sequence[stub_reloc_idx[i]].r_type),
				stub_bfd, stub_sec, stub_sec->contents,
				stub_entry->stub_offset + stub_reloc_offset[i],
				sym_value + stub_entry->target_addend,
				template_sequence[stub_reloc_idx[i]].reloc_addend);
    }

  return true;
#undef MAXRELOCS
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  We also set up the .plt entries for statically linked PIC
   functions here.  This function is called via arm_elf_finish in the
   linker.  */

bool
elf32_csky_build_stubs (struct bfd_link_info *info)
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct csky_elf_link_hash_table *htab;

  htab = csky_elf_hash_table (info);

  if (htab == NULL)
    return false;

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Ignore non-stub sections.  */
      if (!strstr (stub_sec->name, STUB_SUFFIX))
	continue;

      /* Allocate memory to hold the linker stubs.  */
      size = stub_sec->size;
      stub_sec->contents = bfd_zalloc (htab->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return false;
      stub_sec->size = 0;
    }

  /* Build the stubs as directed by the stub hash table.  */
  table = &htab->stub_hash_table;
  bfd_hash_traverse (table, csky_build_one_stub, info);

  return true;
}

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf32_csky_setup_section_lists (bfd *output_bfd,
				struct bfd_link_info *info)
{
  bfd *input_bfd;
  unsigned int bfd_count;
  unsigned int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  size_t amt;
  struct csky_elf_link_hash_table *htab = csky_elf_hash_table (info);

  if (!htab)
    return 0;

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	if (top_id < section->id)
	  top_id = section->id;
    }
  htab->bfd_count = bfd_count;
  amt = sizeof (struct map_stub) * (top_id + 1);
  htab->stub_group = bfd_zmalloc (amt);
  if (htab->stub_group == NULL)
    return -1;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     _bfd_strip_section_from_output doesn't renumber the indices.  */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    if (top_index < section->index)
      top_index = section->index;
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
    if ((section->flags & SEC_CODE) != 0)
      input_list[section->index] = NULL;

  return 1;
}

static bfd_reloc_status_type
csky_relocate_contents (reloc_howto_type *howto,
			bfd *input_bfd,
			bfd_vma relocation,
			bfd_byte *location)
{
  int size;
  bfd_vma x = 0;
  bfd_reloc_status_type flag;
  unsigned int rightshift = howto->rightshift;
  unsigned int bitpos = howto->bitpos;

  if (howto->negate)
    relocation = -relocation;

  /* FIXME: these macros should be defined at file head or head file head.  */
#define CSKY_INSN_ADDI_TO_SUBI        0x04000000
#define CSKY_INSN_MOV_RTB             0xc41d4820   /* mov32 rx, r29, 0 */
#define CSKY_INSN_MOV_RDB             0xc41c4820   /* mov32 rx, r28, 0 */
#define CSKY_INSN_GET_ADDI_RZ(x)      (((x) & 0x03e00000) >> 21)
#define CSKY_INSN_SET_MOV_RZ(x)       ((x) & 0x0000001f)
#define CSKY_INSN_JSRI_TO_LRW         0xea9a0000
#define CSKY_INSN_JSR_R26             0xe8fa0000

  /* Get the value we are going to relocate.  */
  size = bfd_get_reloc_size (howto);
  switch (size)
    {
    default:
    case 0:
      abort ();
    case 1:
      x = bfd_get_8 (input_bfd, location);
      break;
    case 2:
      x = bfd_get_16 (input_bfd, location);
      break;
    case 4:
      if (need_reverse_bits)
	{
	  x = csky_get_insn_32 (input_bfd, location);

	  if (R_CKCORE_DOFFSET_LO16 == howto->type)
	    {
	      if ((bfd_signed_vma) relocation < 0)
		{
		  x |= CSKY_INSN_ADDI_TO_SUBI;
		  relocation = -relocation;
		}
	      else if (0 == relocation)
		x = (CSKY_INSN_MOV_RDB |
		     CSKY_INSN_SET_MOV_RZ (CSKY_INSN_GET_ADDI_RZ (x)));
	    }
	  else if (R_CKCORE_TOFFSET_LO16 == howto->type)
	    {
	      if ((bfd_signed_vma) relocation < 0)
		{
		  x |= CSKY_INSN_ADDI_TO_SUBI;
		  relocation = -relocation;
		}
	      else if (0 == relocation)
		x = (CSKY_INSN_MOV_RTB |
		     CSKY_INSN_SET_MOV_RZ (CSKY_INSN_GET_ADDI_RZ (x)));
	    }
	}
      else
	x = bfd_get_32 (input_bfd, location);
      break;
    }
  /* Check for overflow.  FIXME: We may drop bits during the addition
     which we don't check for.  We must either check at every single
     operation, which would be tedious, or we must do the computations
     in a type larger than bfd_vma, which would be inefficient.  */
  flag = bfd_reloc_ok;
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma addrmask;
      bfd_vma fieldmask;
      bfd_vma signmask;
      bfd_vma ss;
      bfd_vma a;
      bfd_vma b;
      bfd_vma sum;
      /* Get the values to be added together.  For signed and unsigned
	 relocations, we assume that all values should be truncated to
	 the size of an address.  For bitfields, all the bits matter.
	 See also bfd_check_overflow.  */
#define N_ONES(n)      (((((bfd_vma) 1 << ((n) - 1)) - 1) << 1) | 1)
      fieldmask = N_ONES (howto->bitsize);
      signmask  = ~fieldmask;
      addrmask  = N_ONES (bfd_arch_bits_per_address (input_bfd)) | fieldmask;
      a = (relocation & addrmask) >> rightshift;
      if (read_content_substitute)
	x = read_content_substitute;
      b = (x & howto->src_mask & addrmask) >> bitpos;

      switch (howto->complain_on_overflow)
	{
	case complain_overflow_signed:
	  /* If any sign bits are set, all sign bits must be set.
	     That is, A must be a valid negative address after
	     shifting.  */
	  signmask = ~(fieldmask >> 1);
	  /* Fall through.  */

	case complain_overflow_bitfield:
	  /* Much like the signed check, but for a field one bit
	     wider.  We allow a bitfield to represent numbers in the
	     range -2**n to 2**n-1, where n is the number of bits in the
	     field.  Note that when bfd_vma is 32 bits, a 32-bit reloc
	     can't overflow, which is exactly what we want.  */
	  ss = a & signmask;
	  if (ss != 0 && ss != ((addrmask >> rightshift) & signmask))
	    flag = bfd_reloc_overflow;
	  /* We only need this next bit of code if the sign bit of B
	     is below the sign bit of A.  This would only happen if
	     SRC_MASK had fewer bits than BITSIZE.  Note that if
	     SRC_MASK has more bits than BITSIZE, we can get into
	     trouble; we would need to verify that B is in range, as
	     we do for A above.  */
	  ss = ((~howto->src_mask) >> 1) & howto->src_mask;
	  ss >>= bitpos;

	  /* Set all the bits above the sign bit.  */
	  b = (b ^ ss) - ss;

	  /* Now we can do the addition.  */
	  sum = a + b;

	  /* See if the result has the correct sign.  Bits above the
	     sign bit are junk now; ignore them.  If the sum is
	     positive, make sure we did not have all negative inputs;
	     if the sum is negative, make sure we did not have all
	     positive inputs.  The test below looks only at the sign
	     bits, and it really just
	     SIGN (A) == SIGN (B) && SIGN (A) != SIGN (SUM)

	     We mask with addrmask here to explicitly allow an address
	     wrap-around.  The Linux kernel relies on it, and it is
	     the only way to write assembler code which can run when
	     loaded at a location 0x80000000 away from the location at
	     which it is linked.  */

	  if (((~(a ^ b)) & (a ^ sum)) & signmask & addrmask)
	    flag = bfd_reloc_overflow;
	  break;
	case complain_overflow_unsigned:
	  /* Checking for an unsigned overflow is relatively easy:
	     trim the addresses and add, and trim the result as well.
	     Overflow is normally indicated when the result does not
	     fit in the field.  However, we also need to consider the
	     case when, e.g., fieldmask is 0x7fffffff or smaller, an
	     input is 0x80000000, and bfd_vma is only 32 bits; then we
	     will get sum == 0, but there is an overflow, since the
	     inputs did not fit in the field.  Instead of doing a
	     separate test, we can check for this by or-ing in the
	     operands when testing for the sum overflowing its final
	     field.  */
	  sum = (a + b) & addrmask;
	  if ((a | b | sum) & signmask)
	    flag = bfd_reloc_overflow;
	  break;
	default:
	  abort ();
	}

    }
  /* Put RELOCATION in the right bits.  */
  relocation >>= rightshift;

  if ((howto->type == R_CKCORE_DOFFSET_LO16
       || howto->type == R_CKCORE_TOFFSET_LO16)
      && relocation == 0)
    /* Do nothing lsli32 rx, rz, 0.  */
    ;
  else
    {
      /* Fir V1, all this relocation must be x -1.  */
      if (howto->type == R_CKCORE_PCREL_IMM11BY2
	  || howto->type == R_CKCORE_PCREL_JSR_IMM11BY2
	  || howto->type == R_CKCORE_DOFFSET_LO16
	  || howto->type == R_CKCORE_TOFFSET_LO16)
	relocation -= 1;
      else if (howto->type == R_CKCORE_PCREL_IMM7BY4)
	relocation = (relocation & 0x1f) + ((relocation << 3) & 0x300);
      else if (howto->type == R_CKCORE_PCREL_FLRW_IMM8BY4)
	relocation
	  = ((relocation << 4) & 0xf0) + ((relocation << 17) & 0x1e00000);
      else if (howto->type == R_CKCORE_NOJSRI)
	{
	  x = (x & howto->dst_mask) | CSKY_INSN_JSRI_TO_LRW;
	  relocation = 0;
	  csky_put_insn_32 (input_bfd, CSKY_INSN_JSR_R26, location + 4);
	}

      relocation <<= bitpos;
      /* Add RELOCATION to the right bits of X.  */
      x = ((x & ~howto->dst_mask)
	   | (((x & howto->src_mask) + relocation) & howto->dst_mask));
    }
  /* Put the relocated value back in the object file.  */
  switch (size)
    {
    default:
      abort ();
    case 1:
      bfd_put_8 (input_bfd, x, location);
      break;
    case 2:
      bfd_put_16 (input_bfd, x, location);
      break;
    case 4:
      if (need_reverse_bits)
	csky_put_insn_32 (input_bfd, x, location);
      else
	bfd_put_32 (input_bfd, x, location);
      break;
    }
  return flag;
}

/* Look up an entry in the stub hash. Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf32_csky_stub_hash_entry *
elf32_csky_get_stub_entry (const asection *input_section,
			   const asection *sym_sec,
			   struct elf_link_hash_entry *hash,
			   const Elf_Internal_Rela *rel,
			   struct csky_elf_link_hash_table *htab)
{
  struct elf32_csky_stub_hash_entry *stub_entry;
  struct csky_elf_link_hash_entry *h
    = (struct csky_elf_link_hash_entry *) hash;
  const asection *id_sec;

  if ((input_section->flags & SEC_CODE) == 0)
    return NULL;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  id_sec = htab->stub_group[input_section->id].link_sec;
  if (h != NULL && h->stub_cache != NULL
      && h->stub_cache->h == h && h->stub_cache->id_sec == id_sec)
    stub_entry = h->stub_cache;
  else
    {
      char *stub_name;
      stub_name = elf32_csky_stub_name (id_sec, sym_sec, h, rel);
      if (stub_name == NULL)
	return NULL;
      stub_entry = csky_stub_hash_lookup (&htab->stub_hash_table,
					  stub_name, false, false);
      if (h != NULL)
	h->stub_cache = stub_entry;
      free (stub_name);
    }

  return stub_entry;
}

static bfd_reloc_status_type
csky_final_link_relocate (reloc_howto_type *howto,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  bfd_vma address,
			  bfd_vma value,
			  bfd_vma addend)
{
  bfd_vma relocation;

  /* Sanity check the address.  */
  if (address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  /* This function assumes that we are dealing with a basic relocation
     against a symbol. We want to compute the value of the symbol to
     relocate to. This is just VALUE, the value of the symbol,
     plus ADDEND, any addend associated with the reloc.  */
  relocation = value + addend;

  /* If the relocation is PC relative, we want to set RELOCATION to
     the distance between the symbol (currently in RELOCATION) and the
     location we are relocating. Some targets (e.g., i386-aout)
     arrange for the contents of the section to be the negative of the
     offset of the location within the section; for such targets
     pcrel_offset is FALSE.  Other targets (e.g., m88kbcs or ELF)
     simply leave the contents of the section as zero; for such
     targets pcrel_offset is TRUE.  If pcrel_offset is FALSE we do not
     need to subtract out the offset of the location within the
     section (which is just ADDRESS).  */
  if (howto->pc_relative)
    {
      relocation -= (input_section->output_section->vma
		     + input_section->output_offset);
      if (howto->pcrel_offset)
	relocation -= address;
    }

  return csky_relocate_contents (howto, input_bfd, relocation,
				 contents + address);

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

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  bfd_vma base;

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;
  base = align_power ((bfd_vma) TCB_SIZE, htab->tls_sec->alignment_power);
  return address - htab->tls_sec->vma + base;
}

/* Relocate a csky section.  */

static int
csky_elf_relocate_section (bfd *                  output_bfd,
			   struct bfd_link_info * info,
			   bfd *                  input_bfd,
			   asection *             input_section,
			   bfd_byte *             contents,
			   Elf_Internal_Rela *    relocs,
			   Elf_Internal_Sym *     local_syms,
			   asection **            local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  const char *name;
  bool ret = true;
  struct csky_elf_link_hash_table * htab;
  bfd_vma *local_got_offsets = elf_local_got_offsets (input_bfd);

  htab = csky_elf_hash_table (info);
  if (htab == NULL)
    return false;

  symtab_hdr = & elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      enum elf_csky_reloc_type r_type
	= (enum elf_csky_reloc_type) ELF32_R_TYPE (rel->r_info);
      unsigned long r_symndx;
      reloc_howto_type *howto;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma relocation;
      bfd_vma off;
      struct elf_link_hash_entry * h;
      bfd_vma addend = (bfd_vma)rel->r_addend;
      bfd_reloc_status_type r = bfd_reloc_ok;
      bool unresolved_reloc = false;
      int do_final_relocate = true;
      bool relative_reloc = false;
      bfd_signed_vma disp;

      /* Ignore these relocation types:
	 R_CKCORE_GNU_VTINHERIT, R_CKCORE_GNU_VTENTRY.  */
      if (r_type == R_CKCORE_GNU_VTINHERIT || r_type == R_CKCORE_GNU_VTENTRY)
	continue;

      if ((unsigned) r_type >= (unsigned) R_CKCORE_MAX)
	{
	  /* The r_type is error, not support it.  */
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type: %#x"),
			      input_bfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret = false;
	  continue;
	}

      howto = &csky_elf_howto_table[(int) r_type];

      r_symndx = ELF32_R_SYM(rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;
      unresolved_reloc = false;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* Get symbol table entry.  */
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	  addend = (bfd_vma)rel->r_addend;
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
	{
	  /* For relocs against symbols from removed linkonce sections,
	     or sections discarded by a linker script, we just want the
	     section contents zeroed.  Avoid any special processing.
	     And if the symbol is referenced in '.csky_stack_size' section,
	     set the address to SEC_DISCARDED(0xffffffff).  */
#if 0
	  /* The .csky_stack_size section is just for callgraph.  */
	  if (strcmp (input_section->name, ".csky_stack_size") == 0)
	    {
/* FIXME: it should define in head file.  */
#define SEC_DISCARDED   0xffffffff
	      bfd_put_32 (input_bfd, SEC_DISCARDED, contents + rel->r_offset);
	      rel->r_info = 0;
	      rel->r_addend = 0;
	      continue;
	    }
	  else
#endif
	    RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					     rel, 1, relend, howto, 0,
					     contents);
	}

      if (bfd_link_relocatable (info))
	continue;

      read_content_substitute = 0;

      /* Final link.  */
      disp = (relocation
	      + (bfd_signed_vma) addend
	      - input_section->output_section->vma
	      - input_section->output_offset
	      - rel->r_offset);
/* It is for ck8xx.  */
#define CSKY_INSN_BSR32   0xe0000000
/* It is for ck5xx/ck6xx.  */
#define CSKY_INSN_BSR16   0xf800
#define within_range(x, L)  (-(1 << (L - 1)) < (x) && (x) < (1 << (L -1)) - 2)
      switch (howto->type)
	{
	case R_CKCORE_PCREL_IMM18BY2:
	  /* When h is NULL, means the instruction written as
	     grs rx, imm32
	     if the highest bit is set, prevent the high 32bits
	     turn to 0xffffffff when signed extern in 64bit
	     host machine.  */
	  if (h == NULL && (addend & 0x80000000))
	    addend &= 0xffffffff;
	  break;

	case R_CKCORE_PCREL32:
	  break;

	case R_CKCORE_GOT12:
	case R_CKCORE_PLT12:
	case R_CKCORE_GOT_HI16:
	case R_CKCORE_GOT_LO16:
	case R_CKCORE_PLT_HI16:
	case R_CKCORE_PLT_LO16:
	case R_CKCORE_GOT32:
	case R_CKCORE_GOT_IMM18BY4:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  BFD_ASSERT (htab->elf.sgot != NULL);
	  if (h != NULL)
	    {
	      /* Global symbol is defined by other modules.  */
	      bool dyn;
	      off = h->got.offset;
	      dyn = htab->elf.dynamic_sections_created;
	      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						    bfd_link_pic (info), h)
		  || (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info,h))
		  || (ELF_ST_VISIBILITY(h->other)
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
		     When doing a dynamic link, we create a .rela.dyn
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine. FIXME  */
		  if (off & 1)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  htab->elf.sgot->contents + off);
		      h->got.offset |= 1;

/* TRUE if relative relocation should be generated.  GOT reference to
   global symbol in PIC will lead to dynamic symbol.  It becomes a
   problem when "time" or "times" is defined as a variable in an
   executable, clashing with functions of the same name in libc.  If a
   symbol isn't undefined weak symbol, don't make it dynamic in PIC and
   generate relative relocation.  */
#define GENERATE_RELATIVE_RELOC_P(INFO, H) \
  ((H)->dynindx == -1 \
   && !(H)->forced_local \
   && (H)->root.type != bfd_link_hash_undefweak \
   && bfd_link_pic (INFO))

		      if (GENERATE_RELATIVE_RELOC_P (info, h))
			/* If this symbol isn't dynamic
			   in PIC, generate R_CKCORE_RELATIVE here.  */
			relative_reloc = true;
		    }
		}
	      else
		unresolved_reloc = false;
	    } /* End if h != NULL.  */
	  else
	    {
	      BFD_ASSERT (local_got_offsets != NULL);
	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already generated the necessary reloc.  */
	      if (off & 1)
		off &= ~1;
	      else
		{
		  bfd_put_32 (output_bfd, relocation,
			      htab->elf.sgot->contents + off);
		  local_got_offsets[r_symndx] |= 1;
		  if (bfd_link_pic (info))
		    relative_reloc = true;
		}
	    }
	  if (relative_reloc)
	    {
	      asection *srelgot;
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;

	      srelgot = htab->elf.srelgot;
	      BFD_ASSERT (srelgot != NULL);

	      outrel.r_offset
		= (htab->elf.sgot->output_section->vma
		   + htab->elf.sgot->output_offset  + off);
	      outrel.r_info = ELF32_R_INFO (0, R_CKCORE_RELATIVE);
	      outrel.r_addend = relocation;
	      loc = srelgot->contents;
	      loc += (srelgot->reloc_count++ * sizeof (Elf32_External_Rela));
	      if (loc != NULL)
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	    }
	  relocation = htab->elf.sgot->output_offset + off;
	  break;

	case R_CKCORE_GOTOFF_IMM18:
	case R_CKCORE_GOTOFF:
	case R_CKCORE_GOTOFF_HI16:
	case R_CKCORE_GOTOFF_LO16:
	  /* Relocation is relative to the start of the global offset
	     table.  */
	  /* Note that sgot->output_offset is not involved in this
	     calculation.  We always want the start of .got.  If we
	     defined _GLOBAL_OFFSET_TABLE in a different way, as is
	     permitted by the ABI, we might have to change this
	     calculation.  */
	  relocation -= htab->elf.sgot->output_section->vma;
	  break;

	case R_CKCORE_GOTPC:
	case R_CKCORE_GOTPC_HI16:
	case R_CKCORE_GOTPC_LO16:
	  /* Use global offset table as symbol value.  */
	  relocation = htab->elf.sgot->output_section->vma;
	  addend = -addend;
	  unresolved_reloc = false;
	  break;

	case R_CKCORE_DOFFSET_IMM18:
	case R_CKCORE_DOFFSET_IMM18BY2:
	case R_CKCORE_DOFFSET_IMM18BY4:
	  {
	    asection *sdata = bfd_get_section_by_name (output_bfd, ".data");
	    relocation -= sdata->output_section->vma;
	  }
	  break;

	case R_CKCORE_DOFFSET_LO16:
	  {
	    asection *sdata = bfd_get_section_by_name (output_bfd, ".data");
	    relocation -= sdata->output_section->vma;
	  }
	  break;

	case R_CKCORE_TOFFSET_LO16:
	  {
	    asection *stext = bfd_get_section_by_name (output_bfd, ".text");
	    if (stext)
	      relocation -= stext->output_section->vma;
	  }
	  break;

	case R_CKCORE_PLT_IMM18BY4:
	case R_CKCORE_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLT32 reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL)
	    break;

	  if (h->plt.offset == (bfd_vma) -1 || htab->elf.splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      if (h->got.offset != (bfd_vma) -1)
		{
		  bool dyn;

		  off = h->got.offset;
		  dyn = htab->elf.dynamic_sections_created;
		  if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							bfd_link_pic (info), h)
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

			 When doing a dynamic link, we create a .rela.dyn
			 relocation entry to initialize the value.  This
			 is done in the finish_dynamic_symbol routine.
			 FIXME!  */
		      if (off & 1)
			off &= ~1;
		      else
			{
			  h->got.offset |= 1;
			  if (GENERATE_RELATIVE_RELOC_P (info, h))
			    relative_reloc = true;
			}
		    }
		  bfd_put_32 (output_bfd, relocation,
			      htab->elf.sgot->contents + off);

		  if (relative_reloc)
		    {
		      asection *srelgot;
		      Elf_Internal_Rela outrel;
		      bfd_byte *loc;

		      srelgot = htab->elf.srelgot;
		      BFD_ASSERT (srelgot != NULL);

		      outrel.r_offset
			= (htab->elf.sgot->output_section->vma
			   + htab->elf.sgot->output_offset  + off);
		      outrel.r_info = ELF32_R_INFO (0, R_CKCORE_RELATIVE);
		      outrel.r_addend = relocation;
		      loc = srelgot->contents;
		      loc += (srelgot->reloc_count++
			      * sizeof (Elf32_External_Rela));
		      if (loc != NULL)
			bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    }
		  relocation = off + htab->elf.sgot->output_offset;
		}
	      break;
	    }
	  /* The relocation is the got offset.  */
	  if (bfd_csky_abi (output_bfd) == CSKY_ABI_V2)
	    relocation = (h->plt.offset / PLT_ENTRY_SIZE + 2) * 4;
	  else
	    relocation = (h->plt.offset / PLT_ENTRY_SIZE_P + 2) * 4;
	  unresolved_reloc = false;
	  break;

	case R_CKCORE_PCREL_IMM26BY2:
	case R_CKCORE_PCREL_JSR_IMM26BY2:
	case R_CKCORE_PCREL_JSR_IMM11BY2:
	case R_CKCORE_PCREL_IMM11BY2:
	case R_CKCORE_CALLGRAPH:
	  /* Emit callgraph information first.  */
	  /* TODO: deal with callgraph.  */
	  if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_CALLGRAPH)
	    break;
	  /* Some reloc need further handling.  */
	  /* h == NULL means the symbol is a local symbol,
	     r_symndx == 0 means the symbol is 'ABS' and
	     the relocation is already handled in assemble,
	     here just use for callgraph.  */
	  /* TODO: deal with callgraph.  */
	  if (h == NULL && r_symndx == 0)
	    {
	      do_final_relocate = false;
	      break;
	    }

	  /* Ignore weak references to undefined symbols.  */
	  if (h != NULL && h->root.type == bfd_link_hash_undefweak)
	    {
	      do_final_relocate = false;
	      break;
	    }

	  /* Using branch stub.  */
	  if (use_branch_stub == true
	      && ELF32_R_TYPE (rel->r_info) == R_CKCORE_PCREL_IMM26BY2)
	    {
	      struct elf32_csky_stub_hash_entry *stub_entry = NULL;
	      if (sym_must_create_stub (h, info))
		stub_entry = elf32_csky_get_stub_entry (input_section,
							input_section,
							h, rel, htab);
	      else if (disp > BSR_MAX_FWD_BRANCH_OFFSET
		       || disp < BSR_MAX_BWD_BRANCH_OFFSET)
		stub_entry = elf32_csky_get_stub_entry (input_section,
							input_section,
							h, rel, htab);
	      if (stub_entry != NULL)
		relocation
		  = (stub_entry->stub_offset
		     + stub_entry->stub_sec->output_offset
		     + stub_entry->stub_sec->output_section->vma);
	      break;
	    }

	  else if (h == NULL
		   || (h->root.type == bfd_link_hash_defined
		       && h->dynindx == -1)
		   || ((h->def_regular && !h->def_dynamic)
		       && (h->root.type != bfd_link_hash_defweak
			   || ! bfd_link_pic (info))))
	    {
	      if (ELF32_R_TYPE (rel->r_info) == R_CKCORE_PCREL_JSR_IMM26BY2)
		{
		  if (within_range (disp, 26))
		    {
		      /* In range for BSR32.  */
		      howto = &csky_elf_howto_table[R_CKCORE_PCREL_IMM26BY2];
		      read_content_substitute = CSKY_INSN_BSR32;
		    }
		  else if (bfd_csky_arch (output_bfd) == CSKY_ARCH_810)
		    /* if bsr32 cannot reach, generate
		       "lrw r25, label; jsr r25" instead of
		       jsri label.  */
		    howto = &csky_elf_howto_table[R_CKCORE_NOJSRI];
		} /* if ELF32_R_TYPE (rel->r_info)...  */
	      else if (ELF32_R_TYPE (rel->r_info)
		       == R_CKCORE_PCREL_JSR_IMM11BY2)
		{
		  if (within_range (disp, 11))
		    {
		      /* In range for BSR16.  */
		      howto = &csky_elf_howto_table[R_CKCORE_PCREL_IMM11BY2];
		      read_content_substitute = CSKY_INSN_BSR16;
		    }
		}
	      break;
	    } /* else if h == NULL...  */

	  else if (bfd_csky_arch (output_bfd) == CSKY_ARCH_810
		   && (ELF32_R_TYPE (rel->r_info)
		       == R_CKCORE_PCREL_JSR_IMM26BY2))
	    {
	      howto = &csky_elf_howto_table[R_CKCORE_NOJSRI];
	      break;
	    }
	  /* Other situation, h->def_dynamic == 1,
	     undefined_symbol when output file is shared object, etc.  */
	  /* Else fall through.  */

	case R_CKCORE_ADDR_HI16:
	case R_CKCORE_ADDR_LO16:
	  if (bfd_link_pic (info)
	      || (!bfd_link_pic (info)
		  && h != NULL
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && ((h->def_dynamic && !h->def_regular)
		      || (htab->elf.dynamic_sections_created
			  && (h->root.type == bfd_link_hash_undefweak
			      || h->root.type == bfd_link_hash_undefined
			      || h->root.type == bfd_link_hash_indirect)))))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      bfd_byte *loc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at
		 run time.  */
	      skip = false;
	      relocate = false;

	      outrel.r_offset =
		_bfd_elf_section_offset (output_bfd, info, input_section,
					 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1)
		skip = true;
	      else if (outrel.r_offset == (bfd_vma) -2)
		{
		  skip = true;
		  relocate = true;
		}
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);
	      if (skip)
		memset (&outrel, 0, sizeof (outrel));
	      else if (h != NULL
		       && h->dynindx != -1
		       && (!bfd_link_pic (info)
			   || (!SYMBOLIC_BIND (info, h)
			       && h->root.type == bfd_link_hash_defweak)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  relocate = true;
		  outrel.r_info = ELF32_R_INFO (0, r_type);
		  outrel.r_addend = relocation + rel->r_addend;
		}
	      loc = htab->elf.srelgot->contents;
	      loc += (htab->elf.srelgot->reloc_count++
		      * sizeof (Elf32_External_Rela));

	      if (loc != NULL)
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      /* If this reloc is against an external symbol, we do not
		 want to diddle with the addend. Otherwise, we need to
		 include the symbol value so that it becomes an addend
		 for the dynamic reloc.  */
	      if (!relocate)
		continue;
	    } /* if bfd_link_pic (info) ...  */
	  break;

	case R_CKCORE_ADDR32:
	  /* r_symndx will be zero only for relocs against symbols
	     from removed linkonce sections, or sections discarded
	     by a linker script.
	     This relocation don't nedd to handle, the value will
	     be set to SEC_DISCARDED(0xffffffff).  */
	  if (r_symndx == 0
	      && strcmp (sec->name, ".csky_stack_size") == 0)
	    {
	      do_final_relocate = false;
	      break;
	    }
	  if (r_symndx >= symtab_hdr->sh_info
	      && h->non_got_ref
	      && bfd_link_executable (info))
	    break;

	  if (r_symndx == 0 || (input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if (bfd_link_pic (info)
	      || (h != NULL
		  && h->dynindx != -1
		  && ((h->def_dynamic && !h->def_regular)
		      || (htab->elf.dynamic_sections_created
			  && (h->root.type == bfd_link_hash_undefweak
			      || h->root.type == bfd_link_hash_undefined
			      || h->root.type == bfd_link_hash_indirect)))))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      bfd_byte *loc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at
		 run time.  */
	      skip = false;
	      relocate = false;

	      outrel.r_offset =
		_bfd_elf_section_offset (output_bfd, info, input_section,
					 rel->r_offset);

	      if (outrel.r_offset == (bfd_vma) -1)
		skip = true;
	      else if (outrel.r_offset == (bfd_vma) -2)
		{
		  skip = true;
		  relocate = true;
		}

	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		memset (&outrel, 0, sizeof (outrel));
	      else if (h != NULL
		       && h->dynindx != -1
		       && (!bfd_link_pic (info)
			   || (!SYMBOLIC_BIND (info, h)
			       && h->root.type == bfd_link_hash_defweak)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  outrel.r_info = ELF32_R_INFO (0, R_CKCORE_RELATIVE);
		  outrel.r_addend = relocation + rel->r_addend;
		}

	      loc = htab->elf.srelgot->contents;
	      loc += (htab->elf.srelgot->reloc_count++
		      * sizeof (Elf32_External_Rela));

	      if (loc != NULL)
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      /* If this reloc is against an external symbol, we do
		 want to diddle with the addend. Otherwise, we need to
		 include the symbol value so that it becomes an addend
		 for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }
	  break;

	case R_CKCORE_TLS_LDO32:
	  relocation = relocation - dtpoff_base (info);
	  break;

	case R_CKCORE_TLS_LDM32:
	  BFD_ASSERT (htab->elf.sgot != NULL);
	  off = htab->tls_ldm_got.offset;
	  if (off & 1)
	    off &= ~1;
	  else
	    {
	      /* If we don't know the module number,
		 create a relocation for it.  */
	      if (!bfd_link_executable (info))
		{
		  Elf_Internal_Rela outrel;
		  bfd_byte *loc;

		  BFD_ASSERT (htab->elf.srelgot != NULL);
		  outrel.r_addend = 0;
		  outrel.r_offset
		    = (htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset + off);
		  outrel.r_info = ELF32_R_INFO (0, R_CKCORE_TLS_DTPMOD32);
		  bfd_put_32 (output_bfd, outrel.r_addend,
			      htab->elf.sgot->contents + off);

		  loc = htab->elf.srelgot->contents;
		  loc += (htab->elf.srelgot->reloc_count++
			  * sizeof (Elf32_External_Rela));
		  if (loc)
		    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		}
	      else
		bfd_put_32 (output_bfd, 1,
			    htab->elf.sgot->contents + off);
	      htab->tls_ldm_got.offset |= 1;
	    }
	  relocation
	    = (htab->elf.sgot->output_section->vma
	       + htab->elf.sgot->output_offset + off
	       - (input_section->output_section->vma
		  + input_section->output_offset + rel->r_offset));
	  break;
	case R_CKCORE_TLS_LE32:
	  if (bfd_link_dll (info))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA+%#" PRIx64 "): %s relocation not permitted "
		   "in shared object"),
		 input_bfd, input_section, (uint64_t)rel->r_offset,
		 howto->name);
	      return false;
	    }
	  else
	    relocation = tpoff (info, relocation);
	  break;
	case R_CKCORE_TLS_GD32:
	case R_CKCORE_TLS_IE32:
	  {
	    int indx;
	    char tls_type;

	    BFD_ASSERT (htab->elf.sgot != NULL);

	    indx = 0;
	    if (h != NULL)
	      {
		bool dyn;
		dyn = htab->elf.dynamic_sections_created;
		if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info), h)
		    && (!bfd_link_pic (info)
			|| !SYMBOL_REFERENCES_LOCAL (info, h)))
		  {
		    unresolved_reloc = false;
		    indx = h->dynindx;
		  }
		off = h->got.offset;
		tls_type = ((struct csky_elf_link_hash_entry *)h)->tls_type;
	      }
	    else
	      {
		BFD_ASSERT (local_got_offsets != NULL);
		off = local_got_offsets[r_symndx];
		tls_type = csky_elf_local_got_tls_type (input_bfd)[r_symndx];
	      }

	    BFD_ASSERT (tls_type != GOT_UNKNOWN);

	    if (off & 1)
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
		if ((!bfd_link_executable (info) || indx != 0)
		    && (h == NULL
			|| (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			    && !UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
			|| h->root.type != bfd_link_hash_undefined))
		  {
		    need_relocs = true;
		    BFD_ASSERT (htab->elf.srelgot != NULL);

		    loc = htab->elf.srelgot->contents;
		    loc += (htab->elf.srelgot->reloc_count
			    * sizeof (Elf32_External_Rela));
		  }
		if (tls_type & GOT_TLS_GD)
		  {
		    if (need_relocs)
		      {
			outrel.r_addend = 0;
			outrel.r_offset
			  = (htab->elf.sgot->output_section->vma
			     + htab->elf.sgot->output_offset
			     + cur_off);
			outrel.r_info
			  = ELF32_R_INFO (indx, R_CKCORE_TLS_DTPMOD32);
			bfd_put_32 (output_bfd, outrel.r_addend,
				    htab->elf.sgot->contents + cur_off);
			if (loc)
			  bfd_elf32_swap_reloca_out (output_bfd,
						     &outrel, loc);
			loc += sizeof (Elf32_External_Rela);
			htab->elf.srelgot->reloc_count++;
			if (indx == 0)
			  bfd_put_32 (output_bfd,
				      relocation - dtpoff_base (info),
				      (htab->elf.sgot->contents
				       + cur_off + 4));
			else
			  {
			    outrel.r_addend = 0;
			    outrel.r_info
			      = ELF32_R_INFO (indx, R_CKCORE_TLS_DTPOFF32);
			    outrel.r_offset += 4;
			    bfd_put_32 (output_bfd, outrel.r_addend,
					(htab->elf.sgot->contents
					 + cur_off + 4));
			    outrel.r_info =
			      ELF32_R_INFO (indx,
					    R_CKCORE_TLS_DTPOFF32);
			    if (loc)
			      bfd_elf32_swap_reloca_out (output_bfd,
							 &outrel,
							 loc);
			    htab->elf.srelgot->reloc_count++;
			    loc += sizeof (Elf32_External_Rela);
			  }

		      }
		    else
		      {
			/* If are not emitting relocations for a
			   general dynamic reference, then we must be in a
			   static link or an executable link with the
			   symbol binding locally.  Mark it as belonging
			   to module 1, the executable.  */
			bfd_put_32 (output_bfd, 1,
				    htab->elf.sgot->contents + cur_off);
			bfd_put_32 (output_bfd,
				    relocation - dtpoff_base (info),
				    htab->elf.sgot->contents
				    + cur_off + 4);
		      }
		    cur_off += 8;
		  }
		if (tls_type & GOT_TLS_IE)
		  {
		    if (need_relocs)
		      {
			if (indx == 0)
			  outrel.r_addend = relocation - dtpoff_base (info);
			else
			  outrel.r_addend = 0;
			outrel.r_offset
			  = (htab->elf.sgot->output_section->vma
			     + htab->elf.sgot->output_offset + cur_off);
			outrel.r_info
			  = ELF32_R_INFO (indx, R_CKCORE_TLS_TPOFF32);

			bfd_put_32 (output_bfd, outrel.r_addend,
				    htab->elf.sgot->contents + cur_off);
			if (loc)
			  bfd_elf32_swap_reloca_out (output_bfd,
						     &outrel, loc);
			htab->elf.srelgot->reloc_count++;
			loc += sizeof (Elf32_External_Rela);
		      }
		    else
		      bfd_put_32 (output_bfd, tpoff (info, relocation),
				  htab->elf.sgot->contents + cur_off);
		  }
		if (h != NULL)
		  h->got.offset |= 1;
		else
		  local_got_offsets[r_symndx] |= 1;
	      }
	    if ((tls_type & GOT_TLS_GD) && howto->type != R_CKCORE_TLS_GD32)
	      off += 8;
	    relocation
	      = (htab->elf.sgot->output_section->vma
		 + htab->elf.sgot->output_offset + off
		 - (input_section->output_section->vma
		    + input_section->output_offset
		    + rel->r_offset));
	    break;
	  }
	default:
	  /* No substitution when final linking.  */
	  read_content_substitute = 0;
	  break;
	} /* End switch (howto->type).  */

      /* Make sure 32-bit data in the text section will not be affected by
	 our special endianness.
	 However, this currently affects noting, since the ADDR32 howto type
	 does no change with the data read. But we may need this mechanism in
	 the future.  */

      if (bfd_get_reloc_size (howto) == 4
	  && (howto->type == R_CKCORE_ADDR32
	      || howto->type == R_CKCORE_PCREL32
	      || howto->type == R_CKCORE_GOT32
	      || howto->type == R_CKCORE_GOTOFF
	      || howto->type == R_CKCORE_GOTPC
	      || howto->type == R_CKCORE_PLT32
	      || howto->type == R_CKCORE_TLS_LE32
	      || howto->type == R_CKCORE_TLS_IE32
	      || howto->type == R_CKCORE_TLS_LDM32
	      || howto->type == R_CKCORE_TLS_GD32
	      || howto->type == R_CKCORE_TLS_LDO32
	      || howto->type == R_CKCORE_RELATIVE))
	need_reverse_bits = 0;
      else
	need_reverse_bits = 1;
      /* Do the final link.  */
      if (howto->type != R_CKCORE_PCREL_JSR_IMM11BY2
	  && howto->type != R_CKCORE_PCREL_JSR_IMM26BY2
	  && howto->type != R_CKCORE_CALLGRAPH
	  && do_final_relocate)
	r = csky_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation, addend);

      if (r != bfd_reloc_ok)
	{
	  ret = false;
	  switch (r)
	    {
	    default:
	      break;
	    case bfd_reloc_overflow:
	      if (h != NULL)
		name = NULL;
	      else
		{
		  name = bfd_elf_string_from_elf_section (input_bfd,
							  symtab_hdr->sh_link,
							  sym->st_name);
		  if (name == NULL)
		    break;
		  if (*name == '\0')
		    name = bfd_section_name (sec);
		}
	      (*info->callbacks->reloc_overflow)
		(info,
		 (h ? &h->root : NULL),
		 name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;
	    }
	}
    } /* End for (;rel < relend; rel++).  */
  return ret;
}

static bool
csky_elf_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    default:
      return false;
      /* Sizeof (struct elf_prstatus) on C-SKY V1 arch.  */
    case 148:
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);
      offset = 72;
      size = 72;
      break;
      /* Sizeof (struct elf_prstatus) on C-SKY V1 arch.  */
    case 220:
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);
      offset = 72;
      size = 34 * 4;
      break;
    }
  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
csky_elf_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

      /* Sizeof (struct elf_prpsinfo) on linux csky.  */
    case 124:
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

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}

/* Determine whether an object attribute tag takes an integer, a
   string or both.  */

static int
elf32_csky_obj_attrs_arg_type (int tag)
{
  switch (tag)
    {
    case Tag_compatibility:
      return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL;
    case Tag_CSKY_ARCH_NAME:
    case Tag_CSKY_CPU_NAME:
    case Tag_CSKY_FPU_NUMBER_MODULE:
      return ATTR_TYPE_FLAG_STR_VAL;
    case Tag_CSKY_ISA_FLAGS:
    case Tag_CSKY_ISA_EXT_FLAGS:
    case Tag_CSKY_DSP_VERSION:
    case Tag_CSKY_VDSP_VERSION:
    case Tag_CSKY_FPU_VERSION:
    case Tag_CSKY_FPU_ABI:
    case Tag_CSKY_FPU_ROUNDING:
    case Tag_CSKY_FPU_HARDFP:
    case Tag_CSKY_FPU_Exception:
    case Tag_CSKY_FPU_DENORMAL:
      return ATTR_TYPE_FLAG_INT_VAL;
    default:
      break;
    }

  return (tag & 1) != 0 ? ATTR_TYPE_FLAG_STR_VAL : ATTR_TYPE_FLAG_INT_VAL;
}

/* Attribute numbers >=64 (mod 128) can be safely ignored.  */

static bool
elf32_csky_obj_attrs_handle_unknown (bfd *abfd ATTRIBUTE_UNUSED,
				     int tag ATTRIBUTE_UNUSED)
{
  return true;
}

/* End of external entry points for sizing and building linker stubs.  */

/* CPU-related basic API.  */
#define TARGET_BIG_SYM                        csky_elf32_be_vec
#define TARGET_BIG_NAME                       "elf32-csky-big"
#define TARGET_LITTLE_SYM                     csky_elf32_le_vec
#define TARGET_LITTLE_NAME                    "elf32-csky-little"
#define ELF_ARCH                              bfd_arch_csky
#define ELF_MACHINE_CODE                      EM_CSKY
#define ELF_MACHINE_ALT1		      EM_CSKY_OLD
#define ELF_MAXPAGESIZE                       0x1000
#define elf_info_to_howto                     csky_elf_info_to_howto
#define elf_info_to_howto_rel                 NULL
#define elf_backend_special_sections          csky_elf_special_sections
#define bfd_elf32_bfd_link_hash_table_create  csky_elf_link_hash_table_create

/* Target related API.  */
#define bfd_elf32_mkobject                    csky_elf_mkobject
#define bfd_elf32_bfd_merge_private_bfd_data  csky_elf_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags       csky_elf_set_private_flags
#define elf_backend_copy_indirect_symbol      csky_elf_copy_indirect_symbol
#define bfd_elf32_bfd_is_target_special_symbol csky_elf_is_target_special_symbol
#define elf_backend_maybe_function_sym	      csky_elf_maybe_function_sym

/* GC section related API.  */
#define elf_backend_can_gc_sections           1
#define elf_backend_gc_mark_hook              csky_elf_gc_mark_hook
#define elf_backend_gc_mark_extra_sections    elf32_csky_gc_mark_extra_sections

/* Relocation related API.  */
#define elf_backend_reloc_type_class          csky_elf_reloc_type_class
#define bfd_elf32_bfd_reloc_type_lookup       csky_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup       csky_elf_reloc_name_lookup
#define elf_backend_ignore_discarded_relocs   csky_elf_ignore_discarded_relocs
#define elf_backend_relocate_section          csky_elf_relocate_section
#define elf_backend_check_relocs              csky_elf_check_relocs

/* Dynamic relocate related API.  */
#define elf_backend_create_dynamic_sections   _bfd_elf_create_dynamic_sections
#define elf_backend_adjust_dynamic_symbol     csky_elf_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections     csky_elf_size_dynamic_sections
#define elf_backend_finish_dynamic_symbol     csky_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections   csky_elf_finish_dynamic_sections
#define elf_backend_rela_normal               1
#define elf_backend_can_refcount              1
#define elf_backend_plt_readonly              1
#define elf_backend_want_got_sym              1
#define elf_backend_want_dynrelro             1
#define elf_backend_got_header_size           12
#define elf_backend_want_got_plt              1

/* C-SKY coredump support.  */
#define elf_backend_grok_prstatus             csky_elf_grok_prstatus
#define elf_backend_grok_psinfo               csky_elf_grok_psinfo

/* Attribute sections.  */
#undef  elf_backend_obj_attrs_vendor
#define elf_backend_obj_attrs_vendor          "csky"
#undef  elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section         ".csky.attributes"
#undef  elf_backend_obj_attrs_arg_type
#define elf_backend_obj_attrs_arg_type        elf32_csky_obj_attrs_arg_type
#undef  elf_backend_obj_attrs_section_type
#define elf_backend_obj_attrs_section_type    SHT_CSKY_ATTRIBUTES
#define elf_backend_obj_attrs_handle_unknown  elf32_csky_obj_attrs_handle_unknown

#include "elf32-target.h"
