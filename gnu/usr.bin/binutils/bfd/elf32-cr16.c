/* BFD back-end for National Semiconductor's CR16 ELF
   Copyright (C) 2007-2023 Free Software Foundation, Inc.
   Written by M R Swami Reddy.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/cr16.h"
#include "elf32-cr16.h"

/* The cr16 linker needs to keep track of the number of relocs that
   it decides to copy in check_relocs for each symbol.  This is so
   that it can discard PC relative relocs if it doesn't need them when
   linking with -Bsymbolic.  We store the information in a field
   extending the regular ELF linker hash table.  */

struct elf32_cr16_link_hash_entry
{
  /* The basic elf link hash table entry.  */
  struct elf_link_hash_entry root;

  /* For function symbols, the number of times this function is
     called directly (ie by name).  */
  unsigned int direct_calls;

  /* For function symbols, the size of this function's stack
     (if <= 255 bytes).  We stuff this into "call" instructions
     to this target when it's valid and profitable to do so.

     This does not include stack allocated by movm!  */
  unsigned char stack_size;

  /* For function symbols, arguments (if any) for movm instruction
     in the prologue.  We stuff this value into "call" instructions
     to the target when it's valid and profitable to do so.  */
  unsigned char movm_args;

  /* For function symbols, the amount of stack space that would be allocated
     by the movm instruction.  This is redundant with movm_args, but we
     add it to the hash table to avoid computing it over and over.  */
  unsigned char movm_stack_size;

/* Used to mark functions which have had redundant parts of their
   prologue deleted.  */
#define CR16_DELETED_PROLOGUE_BYTES 0x1
  unsigned char flags;

  /* Calculated value.  */
  bfd_vma value;
};

/* cr16_reloc_map array maps BFD relocation enum into a CRGAS relocation type.  */

struct cr16_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_enum; /* BFD relocation enum.  */
  unsigned short cr16_reloc_type;	   /* CR16 relocation type.  */
};

static const struct cr16_reloc_map cr16_reloc_map[R_CR16_MAX] =
{
  {BFD_RELOC_NONE,	     R_CR16_NONE},
  {BFD_RELOC_CR16_NUM8,	     R_CR16_NUM8},
  {BFD_RELOC_CR16_NUM16,     R_CR16_NUM16},
  {BFD_RELOC_CR16_NUM32,     R_CR16_NUM32},
  {BFD_RELOC_CR16_NUM32a,    R_CR16_NUM32a},
  {BFD_RELOC_CR16_REGREL4,   R_CR16_REGREL4},
  {BFD_RELOC_CR16_REGREL4a,  R_CR16_REGREL4a},
  {BFD_RELOC_CR16_REGREL14,  R_CR16_REGREL14},
  {BFD_RELOC_CR16_REGREL14a, R_CR16_REGREL14a},
  {BFD_RELOC_CR16_REGREL16,  R_CR16_REGREL16},
  {BFD_RELOC_CR16_REGREL20,  R_CR16_REGREL20},
  {BFD_RELOC_CR16_REGREL20a, R_CR16_REGREL20a},
  {BFD_RELOC_CR16_ABS20,     R_CR16_ABS20},
  {BFD_RELOC_CR16_ABS24,     R_CR16_ABS24},
  {BFD_RELOC_CR16_IMM4,	     R_CR16_IMM4},
  {BFD_RELOC_CR16_IMM8,	     R_CR16_IMM8},
  {BFD_RELOC_CR16_IMM16,     R_CR16_IMM16},
  {BFD_RELOC_CR16_IMM20,     R_CR16_IMM20},
  {BFD_RELOC_CR16_IMM24,     R_CR16_IMM24},
  {BFD_RELOC_CR16_IMM32,     R_CR16_IMM32},
  {BFD_RELOC_CR16_IMM32a,    R_CR16_IMM32a},
  {BFD_RELOC_CR16_DISP4,     R_CR16_DISP4},
  {BFD_RELOC_CR16_DISP8,     R_CR16_DISP8},
  {BFD_RELOC_CR16_DISP16,    R_CR16_DISP16},
  {BFD_RELOC_CR16_DISP24,    R_CR16_DISP24},
  {BFD_RELOC_CR16_DISP24a,   R_CR16_DISP24a},
  {BFD_RELOC_CR16_SWITCH8,   R_CR16_SWITCH8},
  {BFD_RELOC_CR16_SWITCH16,  R_CR16_SWITCH16},
  {BFD_RELOC_CR16_SWITCH32,  R_CR16_SWITCH32},
  {BFD_RELOC_CR16_GOT_REGREL20, R_CR16_GOT_REGREL20},
  {BFD_RELOC_CR16_GOTC_REGREL20, R_CR16_GOTC_REGREL20},
  {BFD_RELOC_CR16_GLOB_DAT,  R_CR16_GLOB_DAT}
};

static reloc_howto_type cr16_elf_howto_table[] =
{
  HOWTO (R_CR16_NONE,		   /* type */
	 0,			   /* rightshift */
	 0,			   /* size */
	 0,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_dont,   /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_NONE",		   /* name */
	 false,			   /* partial_inplace */
	 0,			   /* src_mask */
	 0,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_NUM8,		   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 8,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_NUM8",		   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xff,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_NUM16,		   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 16,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_NUM16",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_NUM32,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_NUM32",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_NUM32a,		   /* type */
	 1,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_NUM32a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL4,	   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 4,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL4",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xf,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL4a,	   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 4,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL4a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xf,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL14,	   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 14,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL14",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0x3fff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL14a,	   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 14,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL14a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0x3fff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL16,	   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 16,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL16",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL20,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL20",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_REGREL20a,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_REGREL20a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_ABS20,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_ABS20",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_ABS24,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 24,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_ABS24",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM4,		   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 4,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM4",		   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xf,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM8,		   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 8,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM8",		   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xff,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM16,		   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 16,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM16",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM20,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM20",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM24,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 24,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM24",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM32,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM32",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_IMM32a,		   /* type */
	 1,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_IMM32a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_DISP4,		   /* type */
	 1,			   /* rightshift */
	 1,			   /* size */
	 4,			   /* bitsize */
	 true,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_DISP4",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xf,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_DISP8,		   /* type */
	 1,			   /* rightshift */
	 1,			   /* size */
	 8,			   /* bitsize */
	 true,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_DISP8",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0x1ff,			   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_DISP16,		   /* type */
	 0,			   /* rightshift REVIITS: To sync with WinIDEA*/
	 2,			   /* size */
	 16,			   /* bitsize */
	 true,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_DISP16",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0x1ffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */
  /* REVISIT: DISP24 should be left-shift by 2 as per ISA doc
     but its not done, to sync with WinIDEA and CR16 4.1 tools */
  HOWTO (R_CR16_DISP24,		   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 24,			   /* bitsize */
	 true,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_DISP24",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0x1ffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_DISP24a,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 24,			   /* bitsize */
	 true,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_DISP24a",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  /* An 8 bit switch table entry.  This is generated for an expression
     such as ``.byte L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_CR16_SWITCH8,	   /* type */
	 0,			   /* rightshift */
	 1,			   /* size */
	 8,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_SWITCH8",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xff,			   /* dst_mask */
	 true),			   /* pcrel_offset */

  /* A 16 bit switch table entry.  This is generated for an expression
     such as ``.word L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_CR16_SWITCH16,	   /* type */
	 0,			   /* rightshift */
	 2,			   /* size */
	 16,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_SWITCH16",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffff,		   /* dst_mask */
	 true),			   /* pcrel_offset */

  /* A 32 bit switch table entry.  This is generated for an expression
     such as ``.long L1 - L2''.  The offset holds the difference
     between the reloc address and L2.  */
  HOWTO (R_CR16_SWITCH32,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_SWITCH32",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 true),			   /* pcrel_offset */

  HOWTO (R_CR16_GOT_REGREL20,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_GOT_REGREL20",	   /* name */
	 true,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_GOTC_REGREL20,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 20,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_GOTC_REGREL20",   /* name */
	 true,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xfffff,		   /* dst_mask */
	 false),		   /* pcrel_offset */

  HOWTO (R_CR16_GLOB_DAT,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 32,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_unsigned, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	   /* special_function */
	 "R_CR16_GLOB_DAT",	   /* name */
	 false,			   /* partial_inplace */
	 0x0,			   /* src_mask */
	 0xffffffff,		   /* dst_mask */
	 true)			   /* pcrel_offset */
};


/* Create the GOT section.  */

static bool
_bfd_cr16_elf_create_got_section (bfd * abfd, struct bfd_link_info * info)
{
  flagword   flags;
  asection * s;
  struct elf_link_hash_entry * h;
  const struct elf_backend_data * bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);
  int ptralign;

  /* This function may be called more than once.  */
  if (htab->sgot != NULL)
    return true;

  switch (bed->s->arch_size)
    {
    case 16:
      ptralign = 1;
      break;

    case 32:
      ptralign = 2;
      break;

    default:
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  htab->sgot= s;
  if (s == NULL
      || !bfd_set_section_alignment (s, ptralign))
    return false;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      htab->sgotplt = s;
      if (s == NULL
	  || !bfd_set_section_alignment (s, ptralign))
	return false;
    }

  /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
     (or .got.plt) section.  We don't do this in the linker script
     because we don't want to define the symbol if we are not creating
     a global offset table.  */
  h = _bfd_elf_define_linkage_sym (abfd, info, s, "_GLOBAL_OFFSET_TABLE_");
  htab->hgot = h;
  if (h == NULL)
    return false;

  /* The first bit of the global offset table is the header.  */
  s->size += bed->got_header_size;

  return true;
}


/* Retrieve a howto ptr using a BFD reloc_code.  */

static reloc_howto_type *
elf_cr16_reloc_type_lookup (bfd *abfd,
			    bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < R_CR16_MAX; i++)
    if (code == cr16_reloc_map[i].bfd_reloc_enum)
      return &cr16_elf_howto_table[cr16_reloc_map[i].cr16_reloc_type];

  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
		      abfd, code);
  return NULL;
}

static reloc_howto_type *
elf_cr16_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;

  for (i = 0; ARRAY_SIZE (cr16_elf_howto_table); i++)
    if (cr16_elf_howto_table[i].name != NULL
	&& strcasecmp (cr16_elf_howto_table[i].name, r_name) == 0)
      return cr16_elf_howto_table + i;

  return NULL;
}

/* Retrieve a howto ptr using an internal relocation entry.  */

static bool
elf_cr16_info_to_howto (bfd *abfd, arelent *cache_ptr,
			Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  if (r_type >= R_CR16_MAX)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = cr16_elf_howto_table + r_type;
  return true;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
cr16_elf_check_relocs (bfd *abfd, struct bfd_link_info *info, asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Sym * isymbuf = NULL;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bfd *      dynobj;
  bfd_vma *  local_got_offsets;
  asection * sgot;
  asection * srelgot;

  sgot    = NULL;
  srelgot = NULL;
  bool result = false;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  dynobj = elf_hash_table (info)->dynobj;
  local_got_offsets = elf_local_got_offsets (abfd);
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

      /* Some relocs require a global offset table.  */
      if (dynobj == NULL)
	{
	  switch (ELF32_R_TYPE (rel->r_info))
	    {
	    case R_CR16_GOT_REGREL20:
	    case R_CR16_GOTC_REGREL20:
	      elf_hash_table (info)->dynobj = dynobj = abfd;
	      if (! _bfd_cr16_elf_create_got_section (dynobj, info))
		goto fail;
	      break;

	    default:
	      break;
	    }
	}

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_CR16_GOT_REGREL20:
	case R_CR16_GOTC_REGREL20:
	  /* This symbol requires a global offset table entry.  */

	  sgot = elf_hash_table (info)->sgot;
	  srelgot = elf_hash_table (info)->srelgot;
	  BFD_ASSERT (sgot != NULL && srelgot != NULL);

	  if (h != NULL)
	    {
	      if (h->got.offset != (bfd_vma) -1)
		/* We have already allocated space in the .got.  */
		break;

	      h->got.offset = sgot->size;

	      /* Make sure this symbol is output as a dynamic symbol.  */
	      if (h->dynindx == -1)
		{
		  if (! bfd_elf_link_record_dynamic_symbol (info, h))
		    goto fail;
		}

	      srelgot->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    {
	      /* This is a global offset table entry for a local
		 symbol.  */
	      if (local_got_offsets == NULL)
		{
		  size_t       size;
		  unsigned int i;

		  size = symtab_hdr->sh_info * sizeof (bfd_vma);
		  local_got_offsets = (bfd_vma *) bfd_alloc (abfd, size);

		  if (local_got_offsets == NULL)
		    goto fail;

		  elf_local_got_offsets (abfd) = local_got_offsets;

		  for (i = 0; i < symtab_hdr->sh_info; i++)
		    local_got_offsets[i] = (bfd_vma) -1;
		}

	      if (local_got_offsets[r_symndx] != (bfd_vma) -1)
		/* We have already allocated space in the .got.  */
		break;

	      local_got_offsets[r_symndx] = sgot->size;

	      if (bfd_link_executable (info))
		/* If we are generating a shared object, we need to
		   output a R_CR16_RELATIVE reloc so that the dynamic
		   linker can adjust this GOT entry.  */
		srelgot->size += sizeof (Elf32_External_Rela);
	    }

	  sgot->size += 4;
	  break;

	}
    }

  result = true;
 fail:
  free (isymbuf);

  return result;
}

/* Perform a relocation as part of a final link.  */

static bfd_reloc_status_type
cr16_elf_final_link_relocate (reloc_howto_type *howto,
			      bfd *input_bfd,
			      bfd *output_bfd ATTRIBUTE_UNUSED,
			      asection *input_section,
			      bfd_byte *contents,
			      bfd_vma offset,
			      bfd_vma Rvalue,
			      bfd_vma addend,
			      struct elf_link_hash_entry * h,
			      unsigned long symndx  ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info ATTRIBUTE_UNUSED,
			      asection *sec ATTRIBUTE_UNUSED,
			      int is_local ATTRIBUTE_UNUSED)
{
  unsigned short r_type = howto->type;
  bfd_byte *hit_data = contents + offset;
  bfd_vma reloc_bits, check, Rvalue1;

  switch (r_type)
    {
    case R_CR16_IMM4:
    case R_CR16_IMM20:
    case R_CR16_ABS20:
      break;

    case R_CR16_IMM8:
    case R_CR16_IMM16:
    case R_CR16_IMM32:
    case R_CR16_IMM32a:
    case R_CR16_REGREL4:
    case R_CR16_REGREL4a:
    case R_CR16_REGREL14:
    case R_CR16_REGREL14a:
    case R_CR16_REGREL16:
    case R_CR16_REGREL20:
    case R_CR16_REGREL20a:
    case R_CR16_GOT_REGREL20:
    case R_CR16_GOTC_REGREL20:
    case R_CR16_ABS24:
    case R_CR16_DISP16:
    case R_CR16_DISP24:
      /* 'hit_data' is relative to the start of the instruction, not the
	 relocation offset.  Advance it to account for the exact offset.  */
      hit_data += 2;
      break;

    case R_CR16_NONE:
      return bfd_reloc_ok;
      break;

    case R_CR16_DISP4:
      if (is_local)
	Rvalue += -1;
      break;

    case R_CR16_DISP8:
    case R_CR16_DISP24a:
      if (is_local)
	Rvalue -= -1;
      break;

    case R_CR16_SWITCH8:
    case R_CR16_SWITCH16:
    case R_CR16_SWITCH32:
      /* We only care about the addend, where the difference between
	 expressions is kept.  */
      Rvalue = 0;

    default:
      break;
    }

  if (howto->pc_relative)
    {
      /* Subtract the address of the section containing the location.  */
      Rvalue -= (input_section->output_section->vma
		 + input_section->output_offset);
      /* Subtract the position of the location within the section.  */
      Rvalue -= offset;
    }

  /* Add in supplied addend.  */
  Rvalue += addend;

  /* Complain if the bitfield overflows, whether it is considered
     as signed or unsigned.  */
  check = Rvalue >> howto->rightshift;

  reloc_bits = ((bfd_vma) 1 << (howto->bitsize - 1) << 1) - 1;

  /* For GOT and GOTC relocs no boundary checks applied.  */
  if (!((r_type == R_CR16_GOT_REGREL20)
	|| (r_type == R_CR16_GOTC_REGREL20)))
    {
      if (((bfd_vma) check & ~reloc_bits) != 0
	  && (((bfd_vma) check & ~reloc_bits)
	      != (-(bfd_vma) 1 & ~reloc_bits)))
	{
	  /* The above right shift is incorrect for a signed
	     value.  See if turning on the upper bits fixes the
	     overflow.  */
	  if (howto->rightshift && (bfd_signed_vma) Rvalue < 0)
	    {
	      check |= ((bfd_vma) -1
			& ~((bfd_vma) -1 >> howto->rightshift));

	      if (((bfd_vma) check & ~reloc_bits)
		  != (-(bfd_vma) 1 & ~reloc_bits))
		return bfd_reloc_overflow;
	    }
	  else
	    return bfd_reloc_overflow;
	}

      /* Drop unwanted bits from the value we are relocating to.  */
      Rvalue >>= (bfd_vma) howto->rightshift;

      /* Apply dst_mask to select only relocatable part of the insn.  */
      Rvalue &= howto->dst_mask;
    }

  switch (bfd_get_reloc_size (howto))
    {
    case 1:
      if (r_type == R_CR16_DISP8)
	{
	  Rvalue1 = bfd_get_16 (input_bfd, hit_data);
	  Rvalue = ((Rvalue1 & 0xf000) | ((Rvalue << 4) & 0xf00)
		    | (Rvalue1 & 0x00f0) | (Rvalue & 0xf));
	  bfd_put_16 (input_bfd, Rvalue, hit_data);
	}
      else if (r_type == R_CR16_IMM4)
	{
	  Rvalue1 = bfd_get_16 (input_bfd, hit_data);
	  Rvalue = (((Rvalue1 & 0xff) << 8) | ((Rvalue << 4) & 0xf0)
		    | ((Rvalue1 & 0x0f00) >> 8));
	  bfd_put_16 (input_bfd, Rvalue, hit_data);
	}
      else if (r_type == R_CR16_DISP4)
	{
	  Rvalue1 = bfd_get_16 (input_bfd, hit_data);
	  Rvalue = (Rvalue1 | ((Rvalue & 0xf) << 4));
	  bfd_put_16 (input_bfd, Rvalue, hit_data);
	}
      else
	{
	  bfd_put_8 (input_bfd, (unsigned char) Rvalue, hit_data);
	}
      break;

    case 2:
      if (r_type == R_CR16_DISP16)
	{
	  Rvalue |= (bfd_get_16 (input_bfd, hit_data));
	  Rvalue = ((Rvalue & 0xfffe) | ((Rvalue >> 16) & 0x1));
	}
      if (r_type == R_CR16_IMM16)
	{
	  Rvalue1 = bfd_get_16 (input_bfd, hit_data);

	  Rvalue1 = (Rvalue1 ^ 0x8000) - 0x8000;
	  Rvalue += Rvalue1;

	  /* Check for range.  */
	  if (Rvalue > 0xffff)
	    return bfd_reloc_overflow;
	}

      bfd_put_16 (input_bfd, Rvalue, hit_data);
      break;

    case 4:
      if ((r_type == R_CR16_ABS20) || (r_type == R_CR16_IMM20))
	{
	  Rvalue1 = (bfd_get_16 (input_bfd, hit_data + 2)
		     | (((bfd_get_16 (input_bfd, hit_data) & 0xf) << 16)));

	  Rvalue1 = (Rvalue1 ^ 0x80000) - 0x80000;
	  Rvalue += Rvalue1;

	  /* Check for range.  */
	  if (Rvalue > 0xfffff)
	    return bfd_reloc_overflow;

	  bfd_put_16 (input_bfd, ((bfd_get_16 (input_bfd, hit_data) & 0xfff0)
				  | ((Rvalue >> 16) & 0xf)), hit_data);
	  bfd_put_16 (input_bfd, (Rvalue) & 0xffff, hit_data + 2);
	}
      else if (r_type == R_CR16_GOT_REGREL20)
	{
	  asection *sgot = elf_hash_table (info)->sgot;
	  bfd_vma off;

	  if (h != NULL)
	    {
	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) -1);

	      if (! elf_hash_table (info)->dynamic_sections_created
		  || SYMBOL_REFERENCES_LOCAL (info, h))
		/* This is actually a static link, or it is a
		   -Bsymbolic link and the symbol is defined
		   locally, or the symbol was forced to be local
		   because of a version file.  We must initialize
		   this entry in the global offset table.
		   When doing a dynamic link, we create a .rela.got
		   relocation entry to initialize the value.  This
		   is done in the finish_dynamic_symbol routine.  */
		bfd_put_32 (output_bfd, Rvalue, sgot->contents + off);
	    }
	  else
	    {
	      off = elf_local_got_offsets (input_bfd)[symndx];
	      bfd_put_32 (output_bfd, Rvalue, sgot->contents + off);
	    }

	  Rvalue = sgot->output_offset + off;
	  Rvalue += addend;

	  /* REVISIT: if ((long) Rvalue > 0xffffff ||
	     (long) Rvalue < -0x800000).  */
	  if (Rvalue > 0xffffff)
	    return bfd_reloc_overflow;


	  bfd_put_16 (input_bfd, (bfd_get_16 (input_bfd, hit_data))
		      | (((Rvalue >> 16) & 0xf) << 8), hit_data);
	  bfd_put_16 (input_bfd, (Rvalue) & 0xffff, hit_data + 2);

	}
      else if (r_type == R_CR16_GOTC_REGREL20)
	{
	  asection *sgot = elf_hash_table (info)->sgot;
	  bfd_vma off;

	  if (h != NULL)
	    {
	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) -1);

	      Rvalue >>= 1; /* For code symbols.  */

	      if (! elf_hash_table (info)->dynamic_sections_created
		  || SYMBOL_REFERENCES_LOCAL (info, h))
		/* This is actually a static link, or it is a
		   -Bsymbolic link and the symbol is defined
		   locally, or the symbol was forced to be local
		   because of a version file.  We must initialize
		   this entry in the global offset table.
		   When doing a dynamic link, we create a .rela.got
		   relocation entry to initialize the value.  This
		   is done in the finish_dynamic_symbol routine.  */
		bfd_put_32 (output_bfd, Rvalue, sgot->contents + off);
	    }
	  else
	    {
	      off = elf_local_got_offsets (input_bfd)[symndx];
	      Rvalue >>= 1;
	      bfd_put_32 (output_bfd, Rvalue, sgot->contents + off);
	    }

	  Rvalue = sgot->output_offset + off;
	  Rvalue += addend;

	  /* Check if any value in DISP.  */
	  Rvalue1 = bfd_get_32 (input_bfd, hit_data);
	  Rvalue1 = ((Rvalue1 >> 16) | ((Rvalue1 & 0xfff) >> 8 << 16));

	  Rvalue1 = (Rvalue1 ^ 0x80000) - 0x80000;
	  Rvalue += Rvalue1;

	  /* Check for range.  */
	  /* REVISIT: if ((long) Rvalue > 0xffffff
	     || (long) Rvalue < -0x800000).  */
	  if (Rvalue > 0xffffff)
	    return bfd_reloc_overflow;

	  bfd_put_16 (input_bfd, (bfd_get_16 (input_bfd, hit_data))
		      | (((Rvalue >> 16) & 0xf) << 8), hit_data);
	  bfd_put_16 (input_bfd, (Rvalue) & 0xffff, hit_data + 2);
	}
      else
	{
	  if (r_type == R_CR16_ABS24)
	    {
	      Rvalue1 = bfd_get_32 (input_bfd, hit_data);
	      Rvalue1 = ((Rvalue1 >> 16)
			 | ((Rvalue1 & 0xfff) >> 8 << 16)
			 | ((Rvalue1 & 0xf) << 20));

	      Rvalue1 = (Rvalue1 ^ 0x800000) - 0x800000;
	      Rvalue += Rvalue1;

	      /* Check for Range.  */
	      if (Rvalue > 0xffffff)
		return bfd_reloc_overflow;

	      Rvalue = ((((Rvalue >> 20) & 0xf) | (((Rvalue >> 16) & 0xf)<<8)
			 | (bfd_get_32 (input_bfd, hit_data) & 0xf0f0))
			| ((Rvalue & 0xffff) << 16));
	    }
	  else if (r_type == R_CR16_DISP24)
	    {
	      Rvalue = ((((Rvalue >> 20)& 0xf) | (((Rvalue >>16) & 0xf)<<8)
			 | (bfd_get_16 (input_bfd, hit_data)))
			| (((Rvalue & 0xfffe) | ((Rvalue >> 24) & 0x1)) << 16));
	    }
	  else if ((r_type == R_CR16_IMM32) || (r_type == R_CR16_IMM32a))
	    {
	      Rvalue1 = bfd_get_32 (input_bfd, hit_data);
	      Rvalue1 = (((Rvalue1 >> 16) & 0xffff)
			 | ((Rvalue1 & 0xffff) << 16));

	      Rvalue1 = (Rvalue1 ^ 0x80000000) - 0x80000000;
	      Rvalue += Rvalue1;

	      /* Check for range.  */
	      if (Rvalue > 0xffffffff)
		return bfd_reloc_overflow;

	      Rvalue = (((Rvalue >> 16) & 0xffff) | (Rvalue & 0xffff) << 16);
	    }
	  else if (r_type == R_CR16_DISP24a)
	    {
	      Rvalue = (((Rvalue & 0xfffffe) | (Rvalue >> 23)));
	      Rvalue = (((Rvalue >> 16) & 0xff) | ((Rvalue & 0xffff) << 16)
			| bfd_get_32 (input_bfd, hit_data));
	    }
	  else if ((r_type == R_CR16_REGREL20)
		   || (r_type == R_CR16_REGREL20a))
	    {
	      Rvalue1 = bfd_get_32 (input_bfd, hit_data);
	      Rvalue1 = (((Rvalue1 >> 16) & 0xffff)
			 | ((Rvalue1 & 0xfff) >> 8 << 16));

	      Rvalue1 = (Rvalue1 ^ 0x80000) - 0x80000;
	      Rvalue += Rvalue1;

	      /* Check for range.  */
	      if (Rvalue > 0xfffff)
		return bfd_reloc_overflow;

	      Rvalue = (((((Rvalue >> 20) & 0xf) | (((Rvalue >> 16) & 0xf) << 8)
			  | ((Rvalue & 0xffff) << 16)))
			| (bfd_get_32 (input_bfd, hit_data) & 0xf0ff));

	    }
	  else if (r_type == R_CR16_NUM32)
	    {
	      Rvalue1 = (bfd_get_32 (input_bfd, hit_data));

	      Rvalue1 = (Rvalue1 ^ 0x80000000) - 0x80000000;
	      Rvalue += Rvalue1;

	      /* Check for Range.  */
	      if (Rvalue > 0xffffffff)
		return bfd_reloc_overflow;
	    }

	  bfd_put_32 (input_bfd, Rvalue, hit_data);
	}
      break;

    default:
      return bfd_reloc_notsupported;
    }

  return bfd_reloc_ok;
}

/* Delete some bytes from a section while relaxing.  */

static bool
elf32_cr16_relax_delete_bytes (struct bfd_link_info *link_info, bfd *abfd,
			       asection *sec, bfd_vma addr, int count)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel, *irelend;
  bfd_vma toaddr;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  struct elf_link_hash_entry **start_hashes;
  unsigned int symcount;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  contents = elf_section_data (sec)->this_hdr.contents;

  toaddr = sec->size;

  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;

  /* Actually delete the bytes.  */
  memmove (contents + addr, contents + addr + count,
	   (size_t) (toaddr - addr - count));
  sec->size -= count;

  /* Adjust all the relocs.  */
  for (irel = elf_section_data (sec)->relocs; irel < irelend; irel++)
    /* Get the new reloc address.  */
    if ((irel->r_offset > addr && irel->r_offset < toaddr))
      irel->r_offset -= count;

  /* Adjust the local symbols defined in this section.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    {
      if (isym->st_shndx == sec_shndx
	  && isym->st_value > addr
	  && isym->st_value < toaddr)
	{
	  /* Adjust the addend of SWITCH relocations in this section,
	     which reference this local symbol.  */
#if 0
	  for (irel = elf_section_data (sec)->relocs; irel < irelend; irel++)
	    {
	      unsigned long r_symndx;
	      Elf_Internal_Sym *rsym;
	      bfd_vma addsym, subsym;

	      /* Skip if not a SWITCH relocation.  */
	      if (ELF32_R_TYPE (irel->r_info) != (int) R_CR16_SWITCH8
		  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_SWITCH16
		  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_SWITCH32)
		continue;

	      r_symndx = ELF32_R_SYM (irel->r_info);
	      rsym = (Elf_Internal_Sym *) symtab_hdr->contents + r_symndx;

	      /* Skip if not the local adjusted symbol.  */
	      if (rsym != isym)
		continue;

	      addsym = isym->st_value;
	      subsym = addsym - irel->r_addend;

	      /* Fix the addend only when -->> (addsym > addr >= subsym).  */
	      if (subsym <= addr)
		irel->r_addend -= count;
	      else
		continue;
	    }
#endif

	  isym->st_value -= count;
	}
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = start_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;

  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      /* The '--wrap SYMBOL' option is causing a pain when the object file,
	 containing the definition of __wrap_SYMBOL, includes a direct
	 call to SYMBOL as well. Since both __wrap_SYMBOL and SYMBOL reference
	 the same symbol (which is __wrap_SYMBOL), but still exist as two
	 different symbols in 'sym_hashes', we don't want to adjust
	 the global symbol __wrap_SYMBOL twice.
	 This check is only relevant when symbols are being wrapped.  */
      if (link_info->wrap_hash != NULL)
	{
	  struct elf_link_hash_entry **cur_sym_hashes;

	  /* Loop only over the symbols whom been already checked.  */
	  for (cur_sym_hashes = start_hashes; cur_sym_hashes < sym_hashes;
	       cur_sym_hashes++)
	    /* If the current symbol is identical to 'sym_hash', that means
	       the symbol was already adjusted (or at least checked).  */
	    if (*cur_sym_hashes == sym_hash)
	      break;

	  /* Don't adjust the symbol again.  */
	  if (cur_sym_hashes < sym_hashes)
	    continue;
	}

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec
	  && sym_hash->root.u.def.value > addr
	  && sym_hash->root.u.def.value < toaddr)
	sym_hash->root.u.def.value -= count;
    }

  return true;
}

/* Relocate a CR16 ELF section.  */

static int
elf32_cr16_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
			     bfd *input_bfd, asection *input_section,
			     bfd_byte *contents, Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
      howto = cr16_elf_howto_table + (r_type);

      h = NULL;
      sym = NULL;
      sec = NULL;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;

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

      r = cr16_elf_final_link_relocate (howto, input_bfd, output_bfd,
					input_section,
					contents, rel->r_offset,
					relocation, rel->r_addend,
					(struct elf_link_hash_entry *) h,
					r_symndx,
					info, sec, h == NULL);

      if (r != bfd_reloc_ok)
	{
	  const char *name;
	  const char *msg = NULL;

	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = (bfd_elf_string_from_elf_section
		      (input_bfd, symtab_hdr->sh_link, sym->st_name));
	      if (name == NULL || *name == '\0')
		name = bfd_section_name (sec);
	    }

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
	      msg = _("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous error");
	      goto common_error;

	    default:
	      msg = _("internal error: unknown error");
	      /* Fall through.  */

	    common_error:
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      break;
	    }
	}
    }

  return true;
}

/* This is a version of bfd_generic_get_relocated_section_contents
   which uses elf32_cr16_relocate_section.  */

static bfd_byte *
elf32_cr16_get_relocated_section_contents (bfd *output_bfd,
					   struct bfd_link_info *link_info,
					   struct bfd_link_order *link_order,
					   bfd_byte *data,
					   bool relocatable,
					   asymbol **symbols)
{
  Elf_Internal_Shdr *symtab_hdr;
  asection *input_section = link_order->u.indirect.section;
  bfd *input_bfd = input_section->owner;
  asection **sections = NULL;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

  /* We only need to handle the case of relaxing, or of having a
     particular set of section contents, specially.  */
  if (relocatable
      || elf_section_data (input_section)->this_hdr.contents == NULL)
    return bfd_generic_get_relocated_section_contents (output_bfd, link_info,
						       link_order, data,
						       relocatable,
						       symbols);

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;

  bfd_byte *orig_data = data;
  if (data == NULL)
    {
      data = bfd_malloc (input_section->size);
      if (data == NULL)
	return NULL;
    }
  memcpy (data, elf_section_data (input_section)->this_hdr.contents,
	  (size_t) input_section->size);

  if ((input_section->flags & SEC_RELOC) != 0
      && input_section->reloc_count > 0)
    {
      Elf_Internal_Sym *isym;
      Elf_Internal_Sym *isymend;
      asection **secpp;
      bfd_size_type amt;

      internal_relocs = _bfd_elf_link_read_relocs (input_bfd, input_section,
						   NULL, NULL, false);
      if (internal_relocs == NULL)
	goto error_return;

      if (symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    goto error_return;
	}

      amt = symtab_hdr->sh_info;
      amt *= sizeof (asection *);
      sections = bfd_malloc (amt);
      if (sections == NULL && amt != 0)
	goto error_return;

      isymend = isymbuf + symtab_hdr->sh_info;
      for (isym = isymbuf, secpp = sections; isym < isymend; ++isym, ++secpp)
	{
	  asection *isec;

	  if (isym->st_shndx == SHN_UNDEF)
	    isec = bfd_und_section_ptr;
	  else if (isym->st_shndx == SHN_ABS)
	    isec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    isec = bfd_com_section_ptr;
	  else
	    isec = bfd_section_from_elf_index (input_bfd, isym->st_shndx);

	  *secpp = isec;
	}

      if (! elf32_cr16_relocate_section (output_bfd, link_info, input_bfd,
					 input_section, data, internal_relocs,
					 isymbuf, sections))
	goto error_return;

      free (sections);
      if (symtab_hdr->contents != (unsigned char *) isymbuf)
	free (isymbuf);
      if (elf_section_data (input_section)->relocs != internal_relocs)
	free (internal_relocs);
    }

  return data;

 error_return:
  free (sections);
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (input_section)->relocs != internal_relocs)
    free (internal_relocs);
  if (orig_data == NULL)
    free (data);
  return NULL;
}

/* Assorted hash table functions.  */

/* Initialize an entry in the link hash table.  */

/* Create an entry in an CR16 ELF linker hash table.  */

static struct bfd_hash_entry *
elf32_cr16_link_hash_newfunc (struct bfd_hash_entry *entry,
			      struct bfd_hash_table *table,
			      const char *string)
{
  struct elf32_cr16_link_hash_entry *ret =
    (struct elf32_cr16_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct elf32_cr16_link_hash_entry *) NULL)
    ret = ((struct elf32_cr16_link_hash_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct elf32_cr16_link_hash_entry)));
  if (ret == (struct elf32_cr16_link_hash_entry *) NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf32_cr16_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != (struct elf32_cr16_link_hash_entry *) NULL)
    {
      ret->direct_calls = 0;
      ret->stack_size = 0;
      ret->movm_args = 0;
      ret->movm_stack_size = 0;
      ret->flags = 0;
      ret->value = 0;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an cr16 ELF linker hash table.  */

static struct bfd_link_hash_table *
elf32_cr16_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = (struct elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == (struct elf_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (ret, abfd,
				      elf32_cr16_link_hash_newfunc,
				      sizeof (struct elf32_cr16_link_hash_entry),
				      GENERIC_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root;
}

static unsigned long
elf_cr16_mach (flagword flags)
{
  switch (flags)
    {
    case EM_CR16:
    default:
      return bfd_mach_cr16;
    }
}

/* The final processing done just before writing out a CR16 ELF object
   file.  This gets the CR16 architecture right based on the machine
   number.  */

static bool
_bfd_cr16_elf_final_write_processing (bfd *abfd)
{
  unsigned long val;
  switch (bfd_get_mach (abfd))
    {
    default:
    case bfd_mach_cr16:
      val = EM_CR16;
      break;
    }
  elf_elfheader (abfd)->e_flags |= val;
  return _bfd_elf_final_write_processing (abfd);
}


static bool
_bfd_cr16_elf_object_p (bfd *abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_cr16,
			     elf_cr16_mach (elf_elfheader (abfd)->e_flags));
  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
_bfd_cr16_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
      && bfd_get_mach (obfd) < bfd_get_mach (ibfd))
    {
      if (! bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
			       bfd_get_mach (ibfd)))
	return false;
    }

  return true;
}


/* This function handles relaxing for the CR16.

   There's quite a few relaxing opportunites available on the CR16:

	* bcond:24 -> bcond:16				      1 byte
	* bcond:16 -> bcond:8				      1 byte
	* arithmetic imm32 -> arithmetic imm20		      12 bits
	* arithmetic imm20/imm16 -> arithmetic imm4	      12/16 bits

   Symbol- and reloc-reading infrastructure copied from elf-m10200.c.  */

static bool
elf32_cr16_relax_section (bfd *abfd, asection *sec,
			  struct bfd_link_info *link_info, bool *again)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

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

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Get a copy of the native relocations.  */
  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL,
					       link_info->keep_memory);
  if (internal_relocs == NULL)
    goto error_return;

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      bfd_vma symval;

      /* If this isn't something that can be relaxed, then ignore
	 this reloc.  */
      if (ELF32_R_TYPE (irel->r_info) != (int) R_CR16_DISP16
	  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_DISP24
	  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_IMM32
	  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_IMM20
	  && ELF32_R_TYPE (irel->r_info) != (int) R_CR16_IMM16)
	continue;

      /* Get the section contents if we haven't done so already.  */
      if (contents == NULL)
	{
	  /* Get cached copy if it exists.  */
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  /* Go get them off disk.  */
	  else if (!bfd_malloc_and_get_section (abfd, sec, &contents))
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
		    + sym_sec->output_section->vma
		    + sym_sec->output_offset);
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
	}

      /* For simplicity of coding, we are going to modify the section
	 contents, the section relocs, and the BFD symbol table.  We
	 must tell the rest of the code not to free up this
	 information.  It would be possible to instead create a table
	 of changes which have to be made, as is done in coff-mips.c;
	 that would be more work, but would require less memory when
	 the linker is run.  */

      /* Try to turn a 24  branch/call into a 16bit relative
	 branch/call.  */
      if (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_DISP24)
	{
	  bfd_vma value = symval;

	  /* Deal with pc-relative gunk.  */
	  value -= (sec->output_section->vma + sec->output_offset);
	  value -= irel->r_offset;
	  value += irel->r_addend;

	  /* See if the value will fit in 16 bits, note the high value is
	     0xfffe + 2 as the target will be two bytes closer if we are
	     able to relax.  */
	  if ((long) value < 0x10000 && (long) value > -0x10002)
	    {
	      unsigned int code;

	      /* Get the opcode.  */
	      code = (unsigned int) bfd_get_32 (abfd,
						contents + irel->r_offset);

	      /* Verify it's a 'bcond' and fix the opcode.  */
	      if ((code  & 0xffff) == 0x0010)
		bfd_put_16 (abfd, 0x1800 | ((0xf & (code >> 20)) << 4),
			    contents + irel->r_offset);
	      else
		continue;

	      /* Note that we've changed the relocs, section contents, etc.  */
	      elf_section_data (sec)->relocs = internal_relocs;
	      elf_section_data (sec)->this_hdr.contents = contents;
	      symtab_hdr->contents = (unsigned char *) isymbuf;

	      /* Fix the relocation's type.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_CR16_DISP16);

	      /* Delete two bytes of data.  */
	      if (!elf32_cr16_relax_delete_bytes (link_info, abfd, sec,
						  irel->r_offset + 2, 2))
		goto error_return;

	      /* That will change things, so, we should relax again.
		 Note that this is not required, and it may be slow.  */
	      *again = true;
	    }
	}

      /* Try to turn a 16bit pc-relative branch into an
	 8bit pc-relative branch.  */
      if (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_DISP16)
	{
	  bfd_vma value = symval;

	  /* Deal with pc-relative gunk.  */
	  value -= (sec->output_section->vma + sec->output_offset);
	  value -= irel->r_offset;
	  value += irel->r_addend;

	  /* See if the value will fit in 8 bits, note the high value is
	     0xfc + 2 as the target will be two bytes closer if we are
	     able to relax.  */
	  /*if ((long) value < 0x1fa && (long) value > -0x100) REVISIT:range */
	  if ((long) value < 0xfa && (long) value > -0x100)
	    {
	      unsigned short code;

	      /* Get the opcode.  */
	      code = bfd_get_16 (abfd, contents + irel->r_offset);

	      /* Verify it's a 'bcond' and fix the opcode.  */
	      if ((code & 0xff0f) == 0x1800)
		bfd_put_16 (abfd, (code & 0xf0f0), contents + irel->r_offset);
	      else
		continue;

	      /* Note that we've changed the relocs, section contents, etc.  */
	      elf_section_data (sec)->relocs = internal_relocs;
	      elf_section_data (sec)->this_hdr.contents = contents;
	      symtab_hdr->contents = (unsigned char *) isymbuf;

	      /* Fix the relocation's type.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_CR16_DISP8);

	      /* Delete two bytes of data.  */
	      if (!elf32_cr16_relax_delete_bytes (link_info, abfd, sec,
						  irel->r_offset + 2, 2))
		goto error_return;

	      /* That will change things, so, we should relax again.
		 Note that this is not required, and it may be slow.  */
	      *again = true;
	    }
	}

      /* Try to turn a 32-bit IMM address into a 20/16-bit IMM address */
      if (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_IMM32)
	{
	  bfd_vma value = symval;
	  unsigned short is_add_mov = 0;
	  bfd_vma value1 = 0;

	  /* Get the existing value from the mcode */
	  value1 = bfd_get_32 (abfd, contents + irel->r_offset + 2);
	  value1 = (value1 >> 16) | ((value1 & 0xffff) << 16);

	  /* See if the value will fit in 20 bits.  */
	  if ((long) (value + value1) < 0xfffff && (long) (value + value1) > 0)
	    {
	      unsigned short code;

	      /* Get the opcode.  */
	      code = bfd_get_16 (abfd, contents + irel->r_offset);

	      /* Verify it's a 'arithmetic ADDD or MOVD instruction'.
		 For ADDD and MOVD only, convert to IMM32 -> IMM20.  */

	      if (((code & 0xfff0) == 0x0070) || ((code & 0xfff0) == 0x0020))
		is_add_mov = 1;

	      if (is_add_mov)
		{
		  /* Note that we've changed the relocs, section contents,
		     etc.  */
		  elf_section_data (sec)->relocs = internal_relocs;
		  elf_section_data (sec)->this_hdr.contents = contents;
		  symtab_hdr->contents = (unsigned char *) isymbuf;

		  /* Fix the opcode.  */
		  if ((code & 0xfff0) == 0x0070) /* For movd.  */
		    bfd_put_8 (abfd, 0x05, contents + irel->r_offset + 1);
		  else				 /* code == 0x0020 for addd.  */
		    bfd_put_8 (abfd, 0x04, contents + irel->r_offset + 1);

		  bfd_put_8 (abfd, (code & 0xf) << 4, contents + irel->r_offset);

		  /* If existing value is nagavive adjust approriately
		     place the 16-20bits (ie 4 bit) in new opcode,
		     as the 0xffffxxxx, the higher 2 byte values removed. */
		  if (value1 & 0x80000000)
		    bfd_put_8 (abfd,
			       (0x0f | (bfd_get_8 (abfd,
						   contents + irel->r_offset))),
			       contents + irel->r_offset);
		  else
		    bfd_put_8 (abfd,
			       (((value1 >> 16) & 0xf)
				| (bfd_get_8 (abfd,
					      contents + irel->r_offset))),
			       contents + irel->r_offset);

		  /* Fix the relocation's type.  */
		  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					       R_CR16_IMM20);

		  /* Delete two bytes of data.  */
		  if (!elf32_cr16_relax_delete_bytes (link_info, abfd, sec,
						      irel->r_offset + 2, 2))
		    goto error_return;

		  /* That will change things, so, we should relax again.
		     Note that this is not required, and it may be slow.  */
		  *again = true;
		}
	    }

	  /* See if the value will fit in 16 bits.  */
	  if ((!is_add_mov)
	      && ((long)(value + value1) < 0x7fff && (long)(value + value1) > 0))
	    {
	      unsigned short code;

	      /* Get the opcode.  */
	      code = bfd_get_16 (abfd, contents + irel->r_offset);

	      /* Note that we've changed the relocs, section contents, etc.  */
	      elf_section_data (sec)->relocs = internal_relocs;
	      elf_section_data (sec)->this_hdr.contents = contents;
	      symtab_hdr->contents = (unsigned char *) isymbuf;

	      /* Fix the opcode.  */
	      if ((code & 0xf0) == 0x70)	  /* For movd.  */
		bfd_put_8 (abfd, 0x54, contents + irel->r_offset + 1);
	      else if ((code & 0xf0) == 0x20)	  /* For addd.  */
		bfd_put_8 (abfd, 0x60, contents + irel->r_offset + 1);
	      else if ((code & 0xf0) == 0x90)	  /* For cmpd.  */
		bfd_put_8 (abfd, 0x56, contents + irel->r_offset + 1);
	      else
		continue;

	      bfd_put_8 (abfd, 0xb0 | (code & 0xf), contents + irel->r_offset);

	      /* If existing value is nagavive adjust approriately
		 place the 12-16bits (ie 4 bit) in new opcode,
		 as the 0xfffffxxx, the higher 2 byte values removed. */
	      if (value1 & 0x80000000)
		bfd_put_8 (abfd,
			   (0x0f | (bfd_get_8 (abfd,
					       contents + irel->r_offset))),
			   contents + irel->r_offset);
	      else
		bfd_put_16 (abfd, value1, contents + irel->r_offset + 2);


	      /* Fix the relocation's type.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_CR16_IMM16);

	      /* Delete two bytes of data.  */
	      if (!elf32_cr16_relax_delete_bytes (link_info, abfd, sec,
						  irel->r_offset + 2, 2))
		goto error_return;

	      /* That will change things, so, we should relax again.
		 Note that this is not required, and it may be slow.  */
	      *again = true;
	    }
	}

#if 0
      /* Try to turn a 16bit immediate address into a 4bit
	 immediate address.  */
      if ((ELF32_R_TYPE (irel->r_info) == (int) R_CR16_IMM20)
	  || (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_IMM16))
	{
	  bfd_vma value = symval;
	  bfd_vma value1 = 0;

	  /* Get the existing value from the mcode */
	  value1 = ((bfd_get_16 (abfd, contents + irel->r_offset + 2) & 0xffff));

	  if (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_IMM20)
	    {
	      value1 |= ((bfd_get_16 (abfd, contents + irel->r_offset + 1)
			  & 0xf000) << 0x4);
	    }

	  /* See if the value will fit in 4 bits.  */
	  if ((((long) (value + value1)) < 0xf)
	      && (((long) (value + value1)) > 0))
	    {
	      unsigned short code;

	      /* Get the opcode.  */
	      code = bfd_get_16 (abfd, contents + irel->r_offset);

	      /* Note that we've changed the relocs, section contents, etc.  */
	      elf_section_data (sec)->relocs = internal_relocs;
	      elf_section_data (sec)->this_hdr.contents = contents;
	      symtab_hdr->contents = (unsigned char *) isymbuf;

	      /* Fix the opcode.  */
	      if (((code & 0x0f00) == 0x0400) || ((code & 0x0f00) == 0x0500))
		{
		  if ((code & 0x0f00) == 0x0400)      /* For movd imm20.  */
		    bfd_put_8 (abfd, 0x60, contents + irel->r_offset);
		  else				      /* For addd imm20.  */
		    bfd_put_8 (abfd, 0x54, contents + irel->r_offset);
		  bfd_put_8 (abfd, (code & 0xf0) >> 4,
			     contents + irel->r_offset + 1);
		}
	      else
		{
		  if ((code & 0xfff0) == 0x56b0)       /*  For cmpd imm16.  */
		    bfd_put_8 (abfd, 0x56, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x54b0)  /*  For movd imm16.  */
		    bfd_put_8 (abfd, 0x54, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x58b0)  /*  For movb imm16.  */
		    bfd_put_8 (abfd, 0x58, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x5Ab0)  /*  For movw imm16.  */
		    bfd_put_8 (abfd, 0x5A, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x60b0)  /*  For addd imm16.  */
		    bfd_put_8 (abfd, 0x60, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x30b0)  /*  For addb imm16.  */
		    bfd_put_8 (abfd, 0x30, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x2Cb0)  /*  For addub imm16.  */
		    bfd_put_8 (abfd, 0x2C, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x32b0)  /*  For adduw imm16.  */
		    bfd_put_8 (abfd, 0x32, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x38b0)  /*  For subb imm16.  */
		    bfd_put_8 (abfd, 0x38, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x3Cb0)  /*  For subcb imm16.  */
		    bfd_put_8 (abfd, 0x3C, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x3Fb0)  /*  For subcw imm16.  */
		    bfd_put_8 (abfd, 0x3F, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x3Ab0)  /*  For subw imm16.  */
		    bfd_put_8 (abfd, 0x3A, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x50b0)  /*  For cmpb imm16.  */
		    bfd_put_8 (abfd, 0x50, contents + irel->r_offset);
		  else if ((code & 0xfff0) == 0x52b0)  /*  For cmpw imm16.  */
		    bfd_put_8 (abfd, 0x52, contents + irel->r_offset);
		  else
		    continue;

		  bfd_put_8 (abfd, (code & 0xf), contents + irel->r_offset + 1);
		}

	      /* Fix the relocation's type.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_CR16_IMM4);

	      /* Delete two bytes of data.  */
	      if (!elf32_cr16_relax_delete_bytes (link_info, abfd, sec,
						  irel->r_offset + 2, 2))
		goto error_return;

	      /* That will change things, so, we should relax again.
		 Note that this is not required, and it may be slow.  */
	      *again = true;
	    }
	}
#endif
    }

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (! link_info->keep_memory)
	free (isymbuf);
      else
	/* Cache the symbols for elf_link_input_bfd.  */
	symtab_hdr->contents = (unsigned char *) isymbuf;
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (! link_info->keep_memory)
	free (contents);
      else
	/* Cache the section contents for elf_link_input_bfd.  */
	elf_section_data (sec)->this_hdr.contents = contents;

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

static asection *
elf32_cr16_gc_mark_hook (asection *sec,
			 struct bfd_link_info *info,
			 Elf_Internal_Rela *rel,
			 struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym)
{
  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Create dynamic sections when linking against a dynamic object.  */

static bool
_bfd_cr16_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  flagword   flags;
  asection * s;
  const struct elf_backend_data * bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);
  int ptralign = 0;

  switch (bed->s->arch_size)
    {
    case 16:
      ptralign = 1;
      break;

    case 32:
      ptralign = 2;
      break;

    default:
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->default_use_rela_p
					   ? ".rela.plt" : ".rel.plt"),
					  flags | SEC_READONLY);
  htab->srelplt = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, ptralign))
    return false;

  if (! _bfd_cr16_elf_create_got_section (abfd, info))
    return false;

  if (bed->want_dynbss)
    {
      /* The .dynbss section is a place to put symbols which are defined
	 by dynamic objects, are referenced by regular objects, and are
	 not functions.  We must allocate space for them in the process
	 image and use a R_*_COPY reloc to tell the dynamic linker to
	 initialize them at run time.  The linker script puts the .dynbss
	 section into the .bss section of the final image.  */
      s = bfd_make_section_anyway_with_flags (abfd, ".dynbss",
					      SEC_ALLOC | SEC_LINKER_CREATED);
      if (s == NULL)
	return false;

      /* The .rel[a].bss section holds copy relocs.  This section is not
	 normally needed.  We need to create it here, though, so that the
	 linker will map it to an output section.  We can't just create it
	 only if we need it, because we will not know whether we need it
	 until we have seen all the input files, and the first time the
	 main linker code calls BFD after examining all the input files
	 (size_dynamic_sections) the input sections have already been
	 mapped to the output sections.  If the section turns out not to
	 be needed, we can discard it later.  We will never need this
	 section when generating a shared object, since they do not use
	 copy relocs.  */
      if (! bfd_link_executable (info))
	{
	  s = bfd_make_section_anyway_with_flags (abfd,
						  (bed->default_use_rela_p
						   ? ".rela.bss" : ".rel.bss"),
						  flags | SEC_READONLY);
	  if (s == NULL
	      || !bfd_set_section_alignment (s, ptralign))
	    return false;
	}
    }

  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
_bfd_cr16_elf_adjust_dynamic_symbol (struct bfd_link_info * info,
				     struct elf_link_hash_entry * h)
{
  bfd * dynobj;
  asection * s;

  dynobj = elf_hash_table (info)->dynobj;

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
      if (! bfd_link_executable (info)
	  && !h->def_dynamic
	  && !h->ref_dynamic)
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a REL32
	     reloc instead.  */
	  BFD_ASSERT (h->needs_plt);
	  return true;
	}

      /* Make sure this symbol is output as a dynamic symbol.  */
      if (h->dynindx == -1)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      /* We also need to make an entry in the .got.plt section, which
	 will be placed in the .got section by the linker script.  */

      s = elf_hash_table (info)->sgotplt;
      BFD_ASSERT (s != NULL);
      s->size += 4;

      /* We also need to make an entry in the .rela.plt section.  */

      s = elf_hash_table (info)->srelplt;
      BFD_ASSERT (s != NULL);
      s->size += sizeof (Elf32_External_Rela);

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
      return true;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_executable (info))
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

  /* We must generate a R_CR16_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection * srel;

      srel = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Set the sizes of the dynamic sections.  */

static bool
_bfd_cr16_elf_size_dynamic_sections (bfd * output_bfd,
				     struct bfd_link_info * info)
{
  bfd * dynobj;
  asection * s;
  bool relocs;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
#if 0
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
#endif
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

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char * name;

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
	      /* Remember whether there are any reloc sections other
		 than .rela.plt.  */
	      if (strcmp (name, ".rela.plt") != 0)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (! startswith (name, ".got")
	       && strcmp (name, ".dynbss") != 0)
	/* It's not one of our sections, so don't allocate space.  */
	continue;

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

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_CR16_NONE reloc
	 instead of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
_bfd_cr16_elf_finish_dynamic_symbol (bfd * output_bfd,
				     struct bfd_link_info * info,
				     struct elf_link_hash_entry * h,
				     Elf_Internal_Sym * sym)
{
  bfd * dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  if (h->got.offset != (bfd_vma) -1)
    {
      asection *	sgot;
      asection *	srel;
      Elf_Internal_Rela rel;

      /* This symbol has an entry in the global offset table.  Set it up.  */

      sgot = elf_hash_table (info)->sgot;
      srel = elf_hash_table (info)->srelgot;
      BFD_ASSERT (sgot != NULL && srel != NULL);

      rel.r_offset = (sgot->output_section->vma
		      + sgot->output_offset
		      + (h->got.offset & ~1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_executable (info)
	  && (info->symbolic || h->dynindx == -1)
	  && h->def_regular)
	{
	  rel.r_info = ELF32_R_INFO (0, R_CR16_GOT_REGREL20);
	  rel.r_addend = (h->root.u.def.value
			  + h->root.u.def.section->output_section->vma
			  + h->root.u.def.section->output_offset);
	}
      else
	{
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
	  rel.r_info = ELF32_R_INFO (h->dynindx, R_CR16_GOT_REGREL20);
	  rel.r_addend = 0;
	}

      bfd_elf32_swap_reloca_out (output_bfd, &rel,
				 (bfd_byte *) ((Elf32_External_Rela *) srel->contents
					       + srel->reloc_count));
      ++ srel->reloc_count;
    }

  if (h->needs_copy)
    {
      asection *	s;
      Elf_Internal_Rela rel;

      /* This symbol needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_CR16_GOT_REGREL20);
      rel.r_addend = 0;
      bfd_elf32_swap_reloca_out (output_bfd, &rel,
				 (bfd_byte *) ((Elf32_External_Rela *) s->contents
					       + s->reloc_count));
      ++ s->reloc_count;
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (h == elf_hash_table (info)->hdynamic
      || h == elf_hash_table (info)->hgot)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
_bfd_cr16_elf_finish_dynamic_sections (bfd * output_bfd,
				       struct bfd_link_info * info)
{
  bfd *      dynobj;
  asection * sgot;
  asection * sdyn;

  dynobj = elf_hash_table (info)->dynobj;

  sgot = elf_hash_table (info)->sgotplt;
  BFD_ASSERT (sgot != NULL);
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
	  asection * s;

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
    }

  elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;

  return true;
}

/* Given a .data.rel section and a .emreloc in-memory section, store
   relocation information into the .emreloc section which can be
   used at runtime to relocate the section.  This is called by the
   linker when the --embedded-relocs switch is used.  This is called
   after the add_symbols entry point has been called for all the
   objects, and before the final_link entry point is called.  */

bool
bfd_cr16_elf32_create_embedded_relocs (bfd *abfd,
				       struct bfd_link_info *info,
				       asection *datasec,
				       asection *relsec,
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
		     (abfd, datasec, NULL, NULL, info->keep_memory));
  if (internal_relocs == NULL)
    goto error_return;

  amt = (bfd_size_type) datasec->reloc_count * 8;
  relsec->contents = (bfd_byte *) bfd_alloc (abfd, amt);
  if (relsec->contents == NULL)
    goto error_return;

  p = relsec->contents;

  irelend = internal_relocs + datasec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++, p += 8)
    {
      asection *targetsec;

      /* We are going to write a four byte longword into the runtime
	 reloc section.  The longword will be the address in the data
	 section which must be relocated.  It is followed by the name
	 of the target section NUL-padded or truncated to 8
	 characters.  */

      /* We can only relocate absolute longword relocs at run time.  */
      if (!((ELF32_R_TYPE (irel->r_info) == (int) R_CR16_NUM32a)
	    || (ELF32_R_TYPE (irel->r_info) == (int) R_CR16_NUM32)))
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
      memset (p + 4, 0, 4);
      if ((ELF32_R_TYPE (irel->r_info) == (int) R_CR16_NUM32a)
	  && (targetsec != NULL) )
	strncpy ((char *) p + 4, targetsec->output_section->name, 4);
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


/* Classify relocation types, such that combreloc can sort them
   properly.  */

static enum elf_reloc_type_class
_bfd_cr16_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
				const asection *rel_sec ATTRIBUTE_UNUSED,
				const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_CR16_GOT_REGREL20:
    case R_CR16_GOTC_REGREL20:
      return reloc_class_relative;
    default:
      return reloc_class_normal;
    }
}

/* Definitions for setting CR16 target vector.  */
#define TARGET_LITTLE_SYM		  cr16_elf32_vec
#define TARGET_LITTLE_NAME		  "elf32-cr16"
#define ELF_ARCH			  bfd_arch_cr16
#define ELF_MACHINE_CODE		  EM_CR16
#define ELF_MACHINE_ALT1		  EM_CR16_OLD
#define ELF_MAXPAGESIZE			  0x1
#define elf_symbol_leading_char		  '_'

#define bfd_elf32_bfd_reloc_type_lookup	  elf_cr16_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	  elf_cr16_reloc_name_lookup
#define elf_info_to_howto		  elf_cr16_info_to_howto
#define elf_info_to_howto_rel		  NULL
#define elf_backend_relocate_section	  elf32_cr16_relocate_section
#define bfd_elf32_bfd_relax_section	  elf32_cr16_relax_section
#define bfd_elf32_bfd_get_relocated_section_contents \
				elf32_cr16_get_relocated_section_contents
#define elf_backend_gc_mark_hook	  elf32_cr16_gc_mark_hook
#define elf_backend_can_gc_sections	  1
#define elf_backend_rela_normal		  1
#define elf_backend_check_relocs	  cr16_elf_check_relocs
/* So we can set bits in e_flags.  */
#define elf_backend_final_write_processing \
				 _bfd_cr16_elf_final_write_processing
#define elf_backend_object_p	 _bfd_cr16_elf_object_p

#define bfd_elf32_bfd_merge_private_bfd_data \
				 _bfd_cr16_elf_merge_private_bfd_data


#define bfd_elf32_bfd_link_hash_table_create \
				  elf32_cr16_link_hash_table_create

#define elf_backend_create_dynamic_sections \
				  _bfd_cr16_elf_create_dynamic_sections
#define elf_backend_adjust_dynamic_symbol \
				  _bfd_cr16_elf_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections \
				  _bfd_cr16_elf_size_dynamic_sections
#define elf_backend_omit_section_dynsym _bfd_elf_omit_section_dynsym_all
#define elf_backend_finish_dynamic_symbol \
				   _bfd_cr16_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
				   _bfd_cr16_elf_finish_dynamic_sections

#define elf_backend_reloc_type_class   _bfd_cr16_elf_reloc_type_class


#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	12
#define elf_backend_dtrel_excludes_plt	1

#include "elf32-target.h"
