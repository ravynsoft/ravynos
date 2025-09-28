/* Freescale XGATE-specific support for 32-bit ELF
   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Contributed by Sean Keys(skeys@ipdatasys.com)

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
#include "elf/xgate.h"
#include "opcode/xgate.h"
#include "libiberty.h"

/* Forward declarations.  */
static bfd_reloc_status_type xgate_elf_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type xgate_elf_special_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

/* Use REL instead of RELA to save space */
#define USE_REL	1

static reloc_howto_type elf_xgate_howto_table[] =
{
  /* This reloc does nothing.  */
  HOWTO (R_XGATE_NONE, /* type */
	 0, /* rightshift */
	 0, /* size */
	 0, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_NONE", /* name */
	 false, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 8 bit absolute relocation.  */
  HOWTO (R_XGATE_8, /* type */
	 0, /* rightshift */
	 1, /* size */
	 8, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_8", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 8 bit PC-rel relocation.  */
  HOWTO (R_XGATE_PCREL_8, /* type */
	 0, /* rightshift */
	 1, /* size */
	 8, /* bitsize */
	 true, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_8", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 true), /* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_XGATE_16, /* type */
	 0, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont /*bitfield */, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_16", /* name */
	 false, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 32 bit absolute relocation.  This one is never used for the
     code relocation.  It's used by gas for -gstabs generation.  */
  HOWTO (R_XGATE_32, /* type */
	 0, /* rightshift */
	 4, /* size */
	 32, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_32", /* name */
	 false, /* partial_inplace */
	 0xffffffff, /* src_mask */
	 0xffffffff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 16 bit PC-rel relocation.  */
  HOWTO (R_XGATE_PCREL_16, /* type */
	 0, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 true, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_16", /* name */
	 false, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 true), /* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_XGATE_GNU_VTINHERIT, /* type */
	 0, /* rightshift */
	 2, /* size */
	 0, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL, /* special_function */
	 "R_XGATE_GNU_VTINHERIT", /* name */
	 false, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 false), /* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_XGATE_GNU_VTENTRY, /* type */
	 0, /* rightshift */
	 2, /* size */
	 0, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_XGATE_GNU_VTENTRY", /* name */
	 false, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 24 bit relocation.  */
  HOWTO (R_XGATE_24, /* type */
	 0, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_LO", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 16-bit low relocation.  */
  HOWTO (R_XGATE_LO16, /* type */
	 8, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_HI", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A page relocation.  */
  HOWTO (R_XGATE_GPAGE, /* type */
	 0, /* rightshift */
	 1, /* size */
	 8, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_special_reloc,/* special_function */
	 "R_XGATE_GPAGE", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 9 bit absolute relocation.   */
  HOWTO (R_XGATE_PCREL_9, /* type */
	 0, /* rightshift */
	 2, /* size */
	 9, /* bitsize */
	 true, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_9", /* name */
	 false, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 true), /* pcrel_offset */

  /* A 8 bit absolute relocation (upper address).  */
  HOWTO (R_XGATE_PCREL_10, /* type */
	 8, /* rightshift */
	 1, /* size */
	 10, /* bitsize */
	 true, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_PCREL_10", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 true), /* pcrel_offset */

  /* A 8 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM8_LO, /* type */
	 0, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_LO", /* name */
	 false, /* partial_inplace */
	 0xffff, /* src_mask */
	 0xffff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 16 bit absolute relocation (upper address).  */
  HOWTO (R_XGATE_IMM8_HI, /* type */
	 8, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM8_HI", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 3 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM3, /* type */
	 8, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM3", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 4 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM4, /* type */
	 8, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM4", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* A 5 bit absolute relocation.  */
  HOWTO (R_XGATE_IMM5, /* type */
	 8, /* rightshift */
	 2, /* size */
	 16, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_XGATE_IMM5", /* name */
	 false, /* partial_inplace */
	 0x00ff, /* src_mask */
	 0x00ff, /* dst_mask */
	 false), /* pcrel_offset */

  /* Mark beginning of a jump instruction (any form).  */
  HOWTO (R_XGATE_RL_JUMP, /* type */
	 0, /* rightshift */
	 2, /* size */
	 0, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_ignore_reloc, /* special_function */
	 "R_XGATE_RL_JUMP", /* name */
	 true, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 true), /* pcrel_offset */

  /* Mark beginning of Gcc relaxation group instruction.  */
  HOWTO (R_XGATE_RL_GROUP, /* type */
	 0, /* rightshift */
	 2, /* size */
	 0, /* bitsize */
	 false, /* pc_relative */
	 0, /* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 xgate_elf_ignore_reloc, /* special_function */
	 "R_XGATE_RL_GROUP", /* name */
	 true, /* partial_inplace */
	 0, /* src_mask */
	 0, /* dst_mask */
	 true), /* pcrel_offset */
};

/* Map BFD reloc types to XGATE ELF reloc types.  */

struct xgate_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct xgate_reloc_map xgate_reloc_map[] =
{
  {BFD_RELOC_NONE, R_XGATE_NONE},
  {BFD_RELOC_8, R_XGATE_8},
  {BFD_RELOC_8_PCREL, R_XGATE_PCREL_8},
  {BFD_RELOC_16_PCREL, R_XGATE_PCREL_16},
  {BFD_RELOC_16, R_XGATE_16},
  {BFD_RELOC_32, R_XGATE_32},

  {BFD_RELOC_VTABLE_INHERIT, R_XGATE_GNU_VTINHERIT},
  {BFD_RELOC_VTABLE_ENTRY, R_XGATE_GNU_VTENTRY},

  {BFD_RELOC_XGATE_LO16, R_XGATE_LO16},
  {BFD_RELOC_XGATE_GPAGE, R_XGATE_GPAGE},
  {BFD_RELOC_XGATE_24, R_XGATE_24},
  {BFD_RELOC_XGATE_PCREL_9, R_XGATE_PCREL_9},
  {BFD_RELOC_XGATE_PCREL_10,  R_XGATE_PCREL_10},
  {BFD_RELOC_XGATE_IMM8_LO, R_XGATE_IMM8_LO},
  {BFD_RELOC_XGATE_IMM8_HI, R_XGATE_IMM8_HI},
  {BFD_RELOC_XGATE_IMM3, R_XGATE_IMM3},
  {BFD_RELOC_XGATE_IMM4, R_XGATE_IMM4},
  {BFD_RELOC_XGATE_IMM5, R_XGATE_IMM5},

  {BFD_RELOC_XGATE_RL_JUMP, R_XGATE_RL_JUMP},
  {BFD_RELOC_XGATE_RL_GROUP, R_XGATE_RL_GROUP},
};

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (xgate_reloc_map); i++)
    if (xgate_reloc_map[i].bfd_reloc_val == code)
      return &elf_xgate_howto_table[xgate_reloc_map[i].elf_reloc_val];

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf_xgate_howto_table); i++)
    if (elf_xgate_howto_table[i].name != NULL
	&& strcasecmp (elf_xgate_howto_table[i].name, r_name) == 0)
      return &elf_xgate_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an XGATE ELF reloc.  */

static bool
xgate_info_to_howto_rel (bfd *abfd,
			 arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_XGATE_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &elf_xgate_howto_table[r_type];
  return true;
}

/* Specific sections:
 - The .page0 is a data section that is mapped in [0x0000..0x00FF].
   Page0 accesses are faster on the M68HC12.
 - The .vectors is the section that represents the interrupt
   vectors.
 - The .xgate section is starts in 0xE08800 or as xgate sees it 0x0800. */
static const struct bfd_elf_special_section elf32_xgate_special_sections[] =
{
  { STRING_COMMA_LEN (".eeprom"), 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".page0"), 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".softregs"), 0, SHT_NOBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".vectors"), 0, SHT_PROGBITS, SHF_ALLOC },
/*{ STRING_COMMA_LEN (".xgate"),    0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  TODO finish this implementation */
  { NULL, 0, 0, 0, 0 }
};

/* Hook called when reading symbols. */

static void
elf32_xgate_backend_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED,
				       asymbol *sym)
{
  /* Mark xgate symbols.  */
  ((elf_symbol_type *) sym)->internal_elf_sym.st_target_internal = 1;
}

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */

static bfd_reloc_status_type
xgate_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			arelent *reloc_entry,
			asymbol *symbol ATTRIBUTE_UNUSED,
			void *data ATTRIBUTE_UNUSED,
			asection *input_section,
			bfd *output_bfd,
			char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
xgate_elf_special_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			 arelent *reloc_entry ATTRIBUTE_UNUSED,
			 asymbol *symbol ATTRIBUTE_UNUSED,
			 void *data ATTRIBUTE_UNUSED,
			 asection *input_section ATTRIBUTE_UNUSED,
			 bfd *output_bfd ATTRIBUTE_UNUSED,
			 char **error_message ATTRIBUTE_UNUSED)
{
  abort ();
}

static bool
_bfd_xgate_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & E_XGATE_I32)
    fprintf (file, _("[abi=32-bit int, "));
  else
    fprintf (file, _("[abi=16-bit int, "));

  if (elf_elfheader (abfd)->e_flags & E_XGATE_F64)
    fprintf (file, _("64-bit double, "));
  else
    fprintf (file, _("32-bit double, "));
  if (elf_elfheader (abfd)->e_flags & EF_XGATE_MACH)
    fprintf (file, _("cpu=XGATE]"));
  else
    fprintf (file, _("error reading cpu type from elf private data"));
  fputc ('\n', file);

  return true;
}

#define ELF_ARCH			     bfd_arch_xgate
#define ELF_MACHINE_CODE		     EM_XGATE

#define ELF_MAXPAGESIZE			     0x1000

#define TARGET_BIG_SYM			     xgate_elf32_vec
#define TARGET_BIG_NAME			     "elf32-xgate"

#define elf_info_to_howto_rel		     xgate_info_to_howto_rel
#define elf_backend_special_sections	     elf32_xgate_special_sections
#define elf_backend_symbol_processing	     elf32_xgate_backend_symbol_processing
#define bfd_elf32_bfd_print_private_bfd_data _bfd_xgate_elf_print_private_bfd_data

#include "elf32-target.h"
