/* 32-bit ELF support for TI PRU.
   Copyright (C) 2014-2023 Free Software Foundation, Inc.
   Contributed by Dimitar Dimitrov <dimitar@dinux.eu>
   Based on elf32-nios2.c

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

/* This file handles TI PRU ELF targets.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elf/pru.h"
#include "opcode/pru.h"
#include "libiberty.h"

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

#define SWAP_VALS(A,B)		      \
  do {				      \
      (A) ^= (B);		      \
      (B) ^= (A);		      \
      (A) ^= (B);		      \
  } while (0)

/* Enable debugging printout at stdout with this variable.  */
static bool debug_relax = false;

/* Forward declarations.  */
static bfd_reloc_status_type pru_elf32_pmem_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type pru_elf32_s10_pcrel_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type pru_elf32_u8_pcrel_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type pru_elf32_ldi32_relocate
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type bfd_elf_pru_diff_relocate
  (bfd *, arelent *, asymbol *, void *,	asection *, bfd *, char **);

/* Target vector.  */
extern const bfd_target pru_elf32_vec;

/* The relocation table used for SHT_REL sections.  */
static reloc_howto_type elf_pru_howto_table_rel[] = {
  /* No relocation.  */
  HOWTO (R_PRU_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_PRU_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_16_PMEM,
	 2,
	 2,			/* short */
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_PRU_16_PMEM",
	 false,
	 0,			/* src_mask */
	 0xffff,
	 false),

  HOWTO (R_PRU_U16_PMEMIMM,
	 2,
	 4,
	 32,
	 false,
	 8,
	 complain_overflow_unsigned,
	 pru_elf32_pmem_relocate,
	 "R_PRU_U16_PMEMIMM",
	 false,
	 0,			/* src_mask */
	 0x00ffff00,
	 false),

  HOWTO (R_PRU_BFD_RELOC_16,
	 0,
	 2,			/* short */
	 16,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_PRU_BFD_RELOC16",
	 false,
	 0,			/* src_mask */
	 0x0000ffff,
	 false),

  /* 16-bit unsigned immediate relocation.  */
  HOWTO (R_PRU_U16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 8,			/* bitpos */
	 complain_overflow_unsigned,	/* complain on overflow */
	 bfd_elf_generic_reloc,	/* special function */
	 "R_PRU_U16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00ffff00,		/* dest_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_32_PMEM,
	 2,
	 4,			/* long */
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 pru_elf32_pmem_relocate,
	 "R_PRU_32_PMEM",
	 false,
	 0,			/* src_mask */
	 0xffffffff,
	 false),

  HOWTO (R_PRU_BFD_RELOC_32,
	 0,
	 4,			/* long */
	 32,
	 false,
	 0,
	 complain_overflow_dont,
	 bfd_elf_generic_reloc,
	 "R_PRU_BFD_RELOC32",
	 false,
	 0,			/* src_mask */
	 0xffffffff,
	 false),

  HOWTO (R_PRU_S10_PCREL,
	 2,
	 4,
	 10,
	 true,
	 0,
	 complain_overflow_bitfield,
	 pru_elf32_s10_pcrel_relocate,
	 "R_PRU_S10_PCREL",
	 false,
	 0,			/* src_mask */
	 0x060000ff,
	 true),

  HOWTO (R_PRU_U8_PCREL,
	 2,
	 4,
	 8,
	 true,
	 0,
	 complain_overflow_unsigned,
	 pru_elf32_u8_pcrel_relocate,
	 "R_PRU_U8_PCREL",
	 false,
	 0,			/* src_mask */
	 0x000000ff,
	 true),

  HOWTO (R_PRU_LDI32,
	 0,			/* rightshift */
	 8,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned, /* complain on overflow */
	 pru_elf32_ldi32_relocate, /* special function */
	 "R_PRU_LDI32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dest_mask */
	 false),		/* pcrel_offset */

  /* GNU-specific relocations.  */
  HOWTO (R_PRU_GNU_BFD_RELOC_8,
	 0,
	 1,			/* byte */
	 8,
	 false,
	 0,
	 complain_overflow_bitfield,
	 bfd_elf_generic_reloc,
	 "R_PRU_BFD_RELOC8",
	 false,
	 0,			/* src_mask */
	 0x000000ff,
	 false),

  HOWTO (R_PRU_GNU_DIFF8,	/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_pru_diff_relocate, /* special_function */
	 "R_PRU_DIFF8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_GNU_DIFF16,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_pru_diff_relocate,/* special_function */
	 "R_PRU_DIFF16",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_GNU_DIFF32,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_pru_diff_relocate,/* special_function */
	 "R_PRU_DIFF32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_GNU_DIFF16_PMEM,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_pru_diff_relocate,/* special_function */
	 "R_PRU_DIFF16_PMEM",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_PRU_GNU_DIFF32_PMEM, /* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_pru_diff_relocate,/* special_function */
	 "R_PRU_DIFF32_PMEM",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

/* Add other relocations here.  */
};

static unsigned char elf_code_to_howto_index[R_PRU_ILLEGAL + 1];

/* Return the howto for relocation RTYPE.  */

static reloc_howto_type *
lookup_howto (unsigned int rtype)
{
  static bool initialized = false;
  int i;
  int howto_tbl_size = (int) (sizeof (elf_pru_howto_table_rel)
			      / sizeof (elf_pru_howto_table_rel[0]));

  if (! initialized)
    {
      initialized = true;
      memset (elf_code_to_howto_index, 0xff,
	      sizeof (elf_code_to_howto_index));
      for (i = 0; i < howto_tbl_size; i++)
	elf_code_to_howto_index[elf_pru_howto_table_rel[i].type] = i;
    }

  if (rtype > R_PRU_ILLEGAL)
    return NULL;
  i = elf_code_to_howto_index[rtype];
  if (i >= howto_tbl_size)
    return NULL;
  return elf_pru_howto_table_rel + i;
}

/* Map for converting BFD reloc types to PRU reloc types.  */

struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_val;
  enum elf_pru_reloc_type elf_val;
};

static const struct elf_reloc_map pru_reloc_map[] =
{
  {BFD_RELOC_NONE, R_PRU_NONE},
  {BFD_RELOC_PRU_16_PMEM, R_PRU_16_PMEM},
  {BFD_RELOC_PRU_U16_PMEMIMM, R_PRU_U16_PMEMIMM},
  {BFD_RELOC_16, R_PRU_BFD_RELOC_16},
  {BFD_RELOC_PRU_U16, R_PRU_U16},
  {BFD_RELOC_PRU_32_PMEM, R_PRU_32_PMEM},
  {BFD_RELOC_32, R_PRU_BFD_RELOC_32},
  {BFD_RELOC_PRU_S10_PCREL, R_PRU_S10_PCREL},
  {BFD_RELOC_PRU_U8_PCREL, R_PRU_U8_PCREL},
  {BFD_RELOC_PRU_LDI32, R_PRU_LDI32},

  {BFD_RELOC_8, R_PRU_GNU_BFD_RELOC_8},
  {BFD_RELOC_PRU_GNU_DIFF8, R_PRU_GNU_DIFF8},
  {BFD_RELOC_PRU_GNU_DIFF16, R_PRU_GNU_DIFF16},
  {BFD_RELOC_PRU_GNU_DIFF32, R_PRU_GNU_DIFF32},
  {BFD_RELOC_PRU_GNU_DIFF16_PMEM, R_PRU_GNU_DIFF16_PMEM},
  {BFD_RELOC_PRU_GNU_DIFF32_PMEM, R_PRU_GNU_DIFF32_PMEM},
};


/* Assorted hash table functions.  */

/* Create an entry in a PRU ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);

  return entry;
}

/* Implement bfd_elf32_bfd_reloc_type_lookup:
   Given a BFD reloc type, return a howto structure.  */

static reloc_howto_type *
pru_elf32_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				   bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (pru_reloc_map); ++i)
    if (pru_reloc_map[i].bfd_val == code)
      return lookup_howto ((unsigned int) pru_reloc_map[i].elf_val);
  return NULL;
}

/* Implement bfd_elf32_bfd_reloc_name_lookup:
   Given a reloc name, return a howto structure.  */

static reloc_howto_type *
pru_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				   const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf_pru_howto_table_rel); i++)
    if (elf_pru_howto_table_rel[i].name
	&& strcasecmp (elf_pru_howto_table_rel[i].name, r_name) == 0)
      return &elf_pru_howto_table_rel[i];

  return NULL;
}

/* Implement elf_info_to_howto:
   Given a ELF32 relocation, fill in a arelent structure.  */

static bool
pru_elf32_info_to_howto (bfd *abfd, arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= R_PRU_ILLEGAL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"), abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = lookup_howto (r_type);
  return cache_ptr->howto != NULL;
}

/* Do the relocations that require special handling.  */
/* Produce a word address for program memory.  Linker scripts will put .text
   at a high offset in order to differentiate it from .data.  So here we also
   mask the high bits of PMEM address.

   But why 1MB when internal Program Memory much smaller? We want to catch
   unintended overflows.

   Why not use (1<<31) as an offset and a mask? Sitara DDRAM usually resides
   there, and users might want to put some shared carveout memory region in
   their linker scripts.  So 0x80000000 might be a valid .data address.

   Note that we still keep and pass down the original howto.  This way we
   can reuse this function for several different relocations.  */
static bfd_reloc_status_type
pru_elf32_do_pmem_relocate (bfd *abfd, reloc_howto_type *howto,
			    asection *input_section,
			    bfd_byte *data, bfd_vma offset,
			    bfd_vma symbol_value, bfd_vma addend)
{
  symbol_value = symbol_value + addend;
  addend = 0;
  symbol_value &= 0x3fffff;
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

/* Direct copy of _bfd_final_link_relocate, but with special
   "fill-in".  This copy-paste mumbo jumbo is only needed because BFD
   cannot deal correctly with non-contiguous bit fields.  */
static bfd_reloc_status_type
pru_elf32_do_s10_pcrel_relocate (bfd *input_bfd, reloc_howto_type *howto,
				 asection *input_section,
				 bfd_byte *contents, bfd_vma address,
				 bfd_vma relocation, bfd_vma addend)
{
  bfd_byte *location;
  bfd_vma x = 0;
  bfd_vma qboff;
  bfd_reloc_status_type flag = bfd_reloc_ok;

  /* Sanity check the address.  */
  if (address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  BFD_ASSERT (howto->pc_relative);
  BFD_ASSERT (howto->pcrel_offset);

  relocation = relocation + addend - (input_section->output_section->vma
		+ input_section->output_offset) - address;

  location = contents + address;

  /* Get the value we are going to relocate.  */
  BFD_ASSERT (bfd_get_reloc_size (howto) == 4);
  x = bfd_get_32 (input_bfd, location);

  qboff = GET_BROFF_SIGNED (x) << howto->rightshift;
  relocation += qboff;

  BFD_ASSERT (howto->complain_on_overflow == complain_overflow_bitfield);

  if (relocation > 2047 && relocation < (bfd_vma)-2048l)
    flag = bfd_reloc_overflow;

  /* Check that target address is word-aligned.  */
  if (relocation & ((1 << howto->rightshift) - 1))
    flag = bfd_reloc_outofrange;

  relocation >>= (bfd_vma) howto->rightshift;

  /* Fill-in the RELOCATION to the right bits of X.  */
  SET_BROFF_URAW (x, relocation);

  bfd_put_32 (input_bfd, x, location);

  return flag;
}

static bfd_reloc_status_type
pru_elf32_do_u8_pcrel_relocate (bfd *abfd, reloc_howto_type *howto,
				asection *input_section,
				bfd_byte *data, bfd_vma offset,
				bfd_vma symbol_value, bfd_vma addend)
{
  bfd_vma relocation;

  BFD_ASSERT (howto->pc_relative);
  BFD_ASSERT (howto->pcrel_offset);

  relocation = symbol_value + addend - (input_section->output_section->vma
		+ input_section->output_offset) - offset;
  relocation >>= howto->rightshift;

  /* 0 and 1 are invalid target labels for LOOP.  We cannot
     encode this info in HOWTO, so catch such cases here.  */
  if (relocation < 2)
      return bfd_reloc_outofrange;

  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

/* Idea and code taken from elf32-d30v.  */
static bfd_reloc_status_type
pru_elf32_do_ldi32_relocate (bfd *abfd, reloc_howto_type *howto,
			     asection *input_section,
			     bfd_byte *data, bfd_vma offset,
			     bfd_vma symbol_value, bfd_vma addend)
{
  bfd_vma relocation;
  bfd_size_type octets = offset * OCTETS_PER_BYTE (abfd, input_section);
  bfd_byte *location;
  unsigned long in1, in2;

  /* A hacked-up version of _bfd_final_link_relocate() follows.  */

  /* Sanity check the address.  */
  if (octets + bfd_get_reloc_size (howto)
      > bfd_get_section_limit_octets (abfd, input_section))
    return bfd_reloc_outofrange;

  /* This function assumes that we are dealing with a basic relocation
     against a symbol.  We want to compute the value of the symbol to
     relocate to.  This is just VALUE, the value of the symbol, plus
     ADDEND, any addend associated with the reloc.  */
  relocation = symbol_value + addend;

  BFD_ASSERT (!howto->pc_relative);

  /* A hacked-up version of _bfd_relocate_contents() follows.  */
  location = data + octets;

  BFD_ASSERT (!howto->pc_relative);

  in1 = bfd_get_32 (abfd, location);
  in2 = bfd_get_32 (abfd, location + 4);

  SET_INSN_FIELD (IMM16, in1, relocation >> 16);
  SET_INSN_FIELD (IMM16, in2, relocation & 0xffff);

  bfd_put_32 (abfd, in1, location);
  bfd_put_32 (abfd, in2, location + 4);

  /* Old GAS and LD versions have a bug, where the two
     LDI instructions are swapped.  Detect such object
     files and bail.  */
  if (GET_INSN_FIELD (RDSEL, in1) != RSEL_31_16)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("error: %pB: old incompatible object file detected"),
			  abfd);
      return bfd_reloc_notsupported;
    }

  return bfd_reloc_ok;
}

/* HOWTO handlers for relocations that require special handling.  */

static bfd_reloc_status_type
pru_elf32_pmem_relocate (bfd *abfd, arelent *reloc_entry,
			 asymbol *symbol, void *data,
			 asection *input_section, bfd *output_bfd,
			 char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  BFD_ASSERT (0);
  return pru_elf32_do_pmem_relocate (abfd, reloc_entry->howto,
				     input_section,
				     data, reloc_entry->address,
				     (symbol->value
				      + symbol->section->output_section->vma
				      + symbol->section->output_offset),
				     reloc_entry->addend);
}

static bfd_reloc_status_type
pru_elf32_s10_pcrel_relocate (bfd *abfd, arelent *reloc_entry,
				 asymbol *symbol, void *data,
				 asection *input_section, bfd *output_bfd,
				 char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  return pru_elf32_do_s10_pcrel_relocate (abfd, reloc_entry->howto,
					  input_section, data,
					  reloc_entry->address,
					  (symbol->value
					   + symbol->section->output_section->vma
					   + symbol->section->output_offset),
					  reloc_entry->addend);
}

static bfd_reloc_status_type
pru_elf32_u8_pcrel_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			     void *data, asection *input_section,
			     bfd *output_bfd,
			     char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  return pru_elf32_do_u8_pcrel_relocate (abfd, reloc_entry->howto,
					 input_section,
					 data, reloc_entry->address,
					 (symbol->value
					  + symbol->section->output_section->vma
					  + symbol->section->output_offset),
					 reloc_entry->addend);
}

static bfd_reloc_status_type
pru_elf32_ldi32_relocate (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			  void *data, asection *input_section,
			  bfd *output_bfd,
			  char **error_message)
{
  /* If this is a relocatable link (output_bfd test tells us), just
     call the generic function.  Any adjustment will be done at final
     link time.  */
  if (output_bfd != NULL)
    return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				  input_section, output_bfd, error_message);

  return pru_elf32_do_ldi32_relocate (abfd, reloc_entry->howto,
				      input_section,
				      data, reloc_entry->address,
				      (symbol->value
				       + symbol->section->output_section->vma
				       + symbol->section->output_offset),
				      reloc_entry->addend);
}


/* Implement elf_backend_relocate_section.  */
static int
pru_elf32_relocate_section (bfd *output_bfd,
			    struct bfd_link_info *info,
			    bfd *input_bfd,
			    asection *input_section,
			    bfd_byte *contents,
			    Elf_Internal_Rela *relocs,
			    Elf_Internal_Sym *local_syms,
			    asection **local_sections)
{
  struct bfd_elf_section_data * esd = elf_section_data (input_section);
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bool is_rel_reloc;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  /* See if we have a REL type relocation.  */
  is_rel_reloc = (esd->rel.hdr != NULL);
  /* Sanity check - only one type of relocation per section.
     FIXME: Theoretically it is possible to have both types,
     but if that happens how can we distinguish between the two ?  */
  BFD_ASSERT (! is_rel_reloc || ! esd->rela.hdr);

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r = bfd_reloc_ok;
      const char *name = NULL;
      const char* msg = (const char*) NULL;
      bool unresolved_reloc;
      bfd_vma addend;

      /* If we are using a REL relocation then the addend should be empty.  */
      BFD_ASSERT (! is_rel_reloc || rel->r_addend == 0);

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto = lookup_howto ((unsigned) ELF32_R_TYPE (rel->r_info));
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
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	}

      if (sec && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      /* Nothing more to do unless this is a final link.  */
      if (bfd_link_relocatable (info))
	continue;

      if (howto)
	{
	  switch (howto->type)
	    {
	    case R_PRU_NONE:
	      /* We don't need to find a value for this symbol.  It's just a
		 marker.  */
	      r = bfd_reloc_ok;
	      break;

	    case R_PRU_U16:
	      if (is_rel_reloc)
		{
		  unsigned long insn;
		  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  addend = GET_INSN_FIELD (IMM16, insn);
		}
	      else
		addend = rel->r_addend;
	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    addend);
	      break;

	    case R_PRU_U16_PMEMIMM:
	    case R_PRU_32_PMEM:
	    case R_PRU_16_PMEM:
	      if (is_rel_reloc && howto->type == R_PRU_U16_PMEMIMM)
		{
		  unsigned long insn;
		  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  addend = GET_INSN_FIELD (IMM16, insn) << 2;
		}
	      else if (is_rel_reloc && howto->type == R_PRU_32_PMEM)
		{
		  addend = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  addend <<= 2;
		}
	      else if (is_rel_reloc && howto->type == R_PRU_16_PMEM)
		{
		  addend = bfd_get_16 (input_bfd, contents + rel->r_offset);
		  addend <<= 2;
		}
	      else
		{
		  BFD_ASSERT (!is_rel_reloc);
		  addend = rel->r_addend;
		}
	      r = pru_elf32_do_pmem_relocate (input_bfd, howto,
						input_section,
						contents, rel->r_offset,
						relocation, addend);
	      break;
	    case R_PRU_S10_PCREL:
	      BFD_ASSERT (! is_rel_reloc);
	      r = pru_elf32_do_s10_pcrel_relocate (input_bfd, howto,
						      input_section,
						      contents,
						      rel->r_offset,
						      relocation,
						      rel->r_addend);
	      break;
	    case R_PRU_U8_PCREL:
	      BFD_ASSERT (! is_rel_reloc);
	      r = pru_elf32_do_u8_pcrel_relocate (input_bfd, howto,
						      input_section,
						      contents,
						      rel->r_offset,
						      relocation,
						      rel->r_addend);
	      break;
	    case R_PRU_LDI32:
	      if (is_rel_reloc)
		{
		  unsigned long in1, in2;
		  in1 = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  in2 = bfd_get_32 (input_bfd, contents + rel->r_offset + 4);
		  addend = (GET_INSN_FIELD (IMM16, in1) << 16)
			    | GET_INSN_FIELD (IMM16, in2);
		}
	      else
		{
		  addend = rel->r_addend;
		}
	      r = pru_elf32_do_ldi32_relocate (input_bfd, howto,
					       input_section,
					       contents,
					       rel->r_offset,
					       relocation,
					       addend);
	      break;
	    case R_PRU_GNU_DIFF8:
	    case R_PRU_GNU_DIFF16:
	    case R_PRU_GNU_DIFF32:
	    case R_PRU_GNU_DIFF16_PMEM:
	    case R_PRU_GNU_DIFF32_PMEM:
	      /* GNU extensions support only rela.  */
	      BFD_ASSERT (! is_rel_reloc);
	      /* Nothing to do here, as contents already contain the
		 diff value.  */
	      r = bfd_reloc_ok;
	      break;

	    case R_PRU_BFD_RELOC_16:
	      if (is_rel_reloc)
		addend = bfd_get_16 (input_bfd, contents + rel->r_offset);
	      else
		addend = rel->r_addend;
	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    addend);
	      break;

	    case R_PRU_BFD_RELOC_32:
	      if (is_rel_reloc)
		addend = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      else
		addend = rel->r_addend;
	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    addend);
	      break;

	    case R_PRU_GNU_BFD_RELOC_8:
	      BFD_ASSERT (! is_rel_reloc);
	      r = _bfd_final_link_relocate (howto, input_bfd,
					    input_section, contents,
					    rel->r_offset, relocation,
					    rel->r_addend);
	      break;

	    default:
	      BFD_ASSERT (0);
	      break;
	    }
	}
      else
	r = bfd_reloc_notsupported;

      if (r != bfd_reloc_ok)
	{
	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      sym->st_name);
	      if (name == NULL || *name == '\0')
		name = bfd_section_name (sec);
	    }

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow) (info, NULL, name,
						  howto->name, (bfd_vma) 0,
						  input_bfd, input_section,
						  rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol) (info, name, input_bfd,
						    input_section,
						    rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      if (msg == NULL)
		msg = _("relocation out of range");
	      break;

	    case bfd_reloc_notsupported:
	      if (msg == NULL)
		msg = _("unsupported relocation");
	      break;

	    case bfd_reloc_dangerous:
	      if (msg == NULL)
		msg = _("dangerous relocation");
	      break;

	    default:
	      if (msg == NULL)
		msg = _("unknown error");
	      break;
	    }

	  if (msg)
	    {
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      return false;
	    }
	}
    }
  return true;
}


/* Perform a diff relocation.  Nothing to do, as the difference value is
   already written into the section's contents.  */

static bfd_reloc_status_type
bfd_elf_pru_diff_relocate (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *reloc_entry ATTRIBUTE_UNUSED,
			   asymbol *symbol ATTRIBUTE_UNUSED,
			   void *data ATTRIBUTE_UNUSED,
			   asection *input_section ATTRIBUTE_UNUSED,
			   bfd *output_bfd ATTRIBUTE_UNUSED,
			   char **error_message ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}


/* Returns whether the relocation type passed is a diff reloc.  */

static bool
elf32_pru_is_diff_reloc (Elf_Internal_Rela *irel)
{
  return (ELF32_R_TYPE (irel->r_info) == R_PRU_GNU_DIFF8
	  || ELF32_R_TYPE (irel->r_info) == R_PRU_GNU_DIFF16
	  || ELF32_R_TYPE (irel->r_info) == R_PRU_GNU_DIFF32
	  || ELF32_R_TYPE (irel->r_info) == R_PRU_GNU_DIFF16_PMEM
	  || ELF32_R_TYPE (irel->r_info) == R_PRU_GNU_DIFF32_PMEM);
}

/* Reduce the diff value written in the section by count if the shrinked
   insn address happens to fall between the two symbols for which this
   diff reloc was emitted.  */

static void
elf32_pru_adjust_diff_reloc_value (bfd *abfd,
				   struct bfd_section *isec,
				   Elf_Internal_Rela *irel,
				   bfd_vma symval,
				   bfd_vma shrinked_insn_address,
				   int count)
{
  unsigned char *reloc_contents = NULL;
  unsigned char *isec_contents = elf_section_data (isec)->this_hdr.contents;
  if (isec_contents == NULL)
  {
    if (! bfd_malloc_and_get_section (abfd, isec, &isec_contents))
      return;

    elf_section_data (isec)->this_hdr.contents = isec_contents;
  }

  reloc_contents = isec_contents + irel->r_offset;

  /* Read value written in object file.  */
  bfd_signed_vma x = 0;
  switch (ELF32_R_TYPE (irel->r_info))
  {
  case R_PRU_GNU_DIFF8:
    {
      x = bfd_get_signed_8 (abfd, reloc_contents);
      break;
    }
  case R_PRU_GNU_DIFF16:
    {
      x = bfd_get_signed_16 (abfd, reloc_contents);
      break;
    }
  case R_PRU_GNU_DIFF32:
    {
      x = bfd_get_signed_32 (abfd, reloc_contents);
      break;
    }
  case R_PRU_GNU_DIFF16_PMEM:
    {
      x = bfd_get_signed_16 (abfd, reloc_contents) * 4;
      break;
    }
  case R_PRU_GNU_DIFF32_PMEM:
    {
      x = bfd_get_signed_32 (abfd, reloc_contents) * 4;
      break;
    }
  default:
    {
      BFD_FAIL ();
    }
  }

  /* For a diff reloc sym1 - sym2 the diff at assembly time (x) is written
     into the object file at the reloc offset.  sym2's logical value is
     symval (<start_of_section>) + reloc addend.  Compute the start and end
     addresses and check if the shrinked insn falls between sym1 and sym2.  */

  bfd_vma end_address = symval + irel->r_addend;
  bfd_vma start_address = end_address - x;

  /* Shrink the absolute DIFF value (get the to labels "closer"
     together), because we have removed data between labels.  */
  if (x < 0)
    {
      x += count;
      /* In case the signed x is negative, restore order.  */
      SWAP_VALS (end_address, start_address);
    }
  else
    {
      x -= count;
    }

  /* Reduce the diff value by count bytes and write it back into section
    contents.  */

  if (shrinked_insn_address >= start_address
      && shrinked_insn_address <= end_address)
  {
    switch (ELF32_R_TYPE (irel->r_info))
    {
    case R_PRU_GNU_DIFF8:
      {
	bfd_put_signed_8 (abfd, x & 0xFF, reloc_contents);
	break;
      }
    case R_PRU_GNU_DIFF16:
      {
	bfd_put_signed_16 (abfd, x & 0xFFFF, reloc_contents);
	break;
      }
    case R_PRU_GNU_DIFF32:
      {
	bfd_put_signed_32 (abfd, x & 0xFFFFFFFF, reloc_contents);
	break;
      }
    case R_PRU_GNU_DIFF16_PMEM:
      {
	bfd_put_signed_16 (abfd, (x / 4) & 0xFFFF, reloc_contents);
	break;
      }
    case R_PRU_GNU_DIFF32_PMEM:
      {
	bfd_put_signed_32 (abfd, (x / 4) & 0xFFFFFFFF, reloc_contents);
	break;
      }
    default:
      {
	BFD_FAIL ();
      }
    }

  }
}

/* Delete some bytes from a section while changing the size of an instruction.
   The parameter "addr" denotes the section-relative offset pointing just
   behind the shrinked instruction. "addr+count" point at the first
   byte just behind the original unshrinked instruction.

   Idea copied from the AVR port.  */

static bool
pru_elf_relax_delete_bytes (bfd *abfd,
			    asection *sec,
			    bfd_vma addr,
			    int count)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel, *irelend;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymbuf = NULL;
  bfd_vma toaddr;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  contents = elf_section_data (sec)->this_hdr.contents;

  toaddr = sec->size;

  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;

  /* Actually delete the bytes.  */
  if (toaddr - addr - count > 0)
    memmove (contents + addr, contents + addr + count,
	     (size_t) (toaddr - addr - count));
  sec->size -= count;

  /* Adjust all the reloc addresses.  */
  for (irel = elf_section_data (sec)->relocs; irel < irelend; irel++)
    {
      bfd_vma old_reloc_address;

      old_reloc_address = (sec->output_section->vma
			   + sec->output_offset + irel->r_offset);

      /* Get the new reloc address.  */
      if ((irel->r_offset > addr
	   && irel->r_offset < toaddr))
	{
	  if (debug_relax)
	    printf ("Relocation at address 0x%x needs to be moved.\n"
		    "Old section offset: 0x%x, New section offset: 0x%x \n",
		    (unsigned int) old_reloc_address,
		    (unsigned int) irel->r_offset,
		    (unsigned int) ((irel->r_offset) - count));

	  irel->r_offset -= count;
	}

    }

   /* The reloc's own addresses are now ok.  However, we need to readjust
      the reloc's addend, i.e. the reloc's value if two conditions are met:
      1.) the reloc is relative to a symbol in this section that
	  is located in front of the shrinked instruction
      2.) symbol plus addend end up behind the shrinked instruction.

      The most common case where this happens are relocs relative to
      the section-start symbol.

      This step needs to be done for all of the sections of the bfd.  */

  {
    struct bfd_section *isec;

    for (isec = abfd->sections; isec; isec = isec->next)
     {
       bfd_vma symval;
       bfd_vma shrinked_insn_address;

       if (isec->reloc_count == 0)
	 continue;

       shrinked_insn_address = (sec->output_section->vma
				+ sec->output_offset + addr);

       irel = elf_section_data (isec)->relocs;
       /* PR 12161: Read in the relocs for this section if necessary.  */
       if (irel == NULL)
	 irel = _bfd_elf_link_read_relocs (abfd, isec, NULL, NULL, true);

       for (irelend = irel + isec->reloc_count;
	    irel < irelend;
	    irel++)
	 {
	   /* Read this BFD's local symbols if we haven't done
	      so already.  */
	   if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	     {
	       isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	       if (isymbuf == NULL)
		 isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
						 symtab_hdr->sh_info, 0,
						 NULL, NULL, NULL);
	       if (isymbuf == NULL)
		 return false;
	     }

	   /* Get the value of the symbol referred to by the reloc.  */
	   if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	     {
	       /* A local symbol.  */
	       asection *sym_sec;

	       isym = isymbuf + ELF32_R_SYM (irel->r_info);
	       sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	       symval = isym->st_value;
	       /* If the reloc is absolute, it will not have
		  a symbol or section associated with it.  */
	       if (sym_sec == sec)
		 {
		   symval += sym_sec->output_section->vma
		     + sym_sec->output_offset;

		   if (debug_relax)
		     printf ("Checking if the relocation's "
			     "addend needs corrections.\n"
			     "Address of anchor symbol: 0x%x \n"
			     "Address of relocation target: 0x%x \n"
			     "Address of relaxed insn: 0x%x \n",
			     (unsigned int) symval,
			     (unsigned int) (symval + irel->r_addend),
			     (unsigned int) shrinked_insn_address);

		   /* Shrink the special DIFF relocations.  */
		   if (elf32_pru_is_diff_reloc (irel))
		     {
		       elf32_pru_adjust_diff_reloc_value (abfd, isec, irel,
							  symval,
							  shrinked_insn_address,
							  count);
		     }

		   /* Fix the addend, if it is affected.  */
		   if (symval <= shrinked_insn_address
		       && (symval + irel->r_addend) > shrinked_insn_address)
		     {

		       irel->r_addend -= count;

		       if (debug_relax)
			 printf ("Relocation's addend needed to be fixed \n");
		     }
		 }
	       /* else...Reference symbol is absolute.
		  No adjustment needed.  */
	     }
	   /* else...Reference symbol is extern.  No need for adjusting
	      the addend.  */
	 }
     }
  }

  /* Adjust the local symbols defined in this section.  */
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  /* Fix PR 9841, there may be no local symbols.  */
  if (isym != NULL)
    {
      Elf_Internal_Sym *isymend;

      isymend = isym + symtab_hdr->sh_info;
      for (; isym < isymend; isym++)
	{
	  if (isym->st_shndx == sec_shndx)
	    {
	      if (isym->st_value > addr
		  && isym->st_value <= toaddr)
		isym->st_value -= count;

	      if (isym->st_value <= addr
		  && isym->st_value + isym->st_size > addr)
		{
		  /* If this assert fires then we have a symbol that ends
		     part way through an instruction.  Does that make
		     sense?  */
		  BFD_ASSERT (isym->st_value + isym->st_size >= addr + count);
		  isym->st_size -= count;
		}
	    }
	}
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
	  && sym_hash->root.u.def.section == sec)
	{
	  if (sym_hash->root.u.def.value > addr
	      && sym_hash->root.u.def.value <= toaddr)
	    sym_hash->root.u.def.value -= count;

	  if (sym_hash->root.u.def.value <= addr
	      && (sym_hash->root.u.def.value + sym_hash->size > addr))
	    {
	      /* If this assert fires then we have a symbol that ends
		 part way through an instruction.  Does that make
		 sense?  */
	      BFD_ASSERT (sym_hash->root.u.def.value + sym_hash->size
			  >= addr + count);
	      sym_hash->size -= count;
	    }
	}
    }

  return true;
}

static bool
pru_elf32_relax_section (bfd *abfd, asection *sec,
			 struct bfd_link_info *link_info,
			 bool *again)
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

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;

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

      /* Check if we can remove an LDI instruction from the LDI32
	 pseudo instruction if the upper 16 operand bits are zero.  */
      if (ELF32_R_TYPE (irel->r_info) == (int) R_PRU_LDI32)
	{
	  bfd_vma value = symval + irel->r_addend;

	  if (debug_relax)
	    printf ("R_PRU_LDI32 with value=0x%lx\n", (long) value);

	  if ((long) value >> 16 == 0)
	    {
	      unsigned long insn;

	      /* Note that we've changed the relocs, section contents.  */
	      elf_section_data (sec)->relocs = internal_relocs;
	      elf_section_data (sec)->this_hdr.contents = contents;
	      symtab_hdr->contents = (unsigned char *) isymbuf;

	      /* Make the second instruction load the 16-bit constant
		 into the full 32-bit register.  */
	      insn = bfd_get_32 (abfd, contents + irel->r_offset + 4);

	      /* Old GAS and LD versions have a bug, where the two
		 LDI instructions are swapped.  Detect such object
		 files and bail.  */
	      if (GET_INSN_FIELD (RDSEL, insn) != RSEL_15_0)
		{
		  /* xgettext:c-format */
		  _bfd_error_handler (_("error: %pB: old incompatible object file detected"),
				      abfd);
		  goto error_return;
		}

	      SET_INSN_FIELD (RDSEL, insn, RSEL_31_0);
	      bfd_put_32 (abfd, insn, contents + irel->r_offset + 4);

	      /* Delete the first LDI instruction.  Note that there should
		 be no relocations or symbols pointing to the second LDI
		 instruction.  */
	      if (!pru_elf_relax_delete_bytes (abfd, sec, irel->r_offset, 4))
		goto error_return;

	      /* We're done with deletion of the first instruction.
		 Set a regular LDI relocation for the second instruction
		 we left to load the 16-bit value into the 32-bit
		 register.  */
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_PRU_U16);

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

/* Free the derived linker hash table.  */
static void
pru_elf32_link_hash_table_free (bfd *obfd)
{
  _bfd_elf_link_hash_table_free (obfd);
}

/* Implement bfd_elf32_bfd_link_hash_table_create.  */
static struct bfd_link_hash_table *
pru_elf32_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (ret, abfd,
				      link_hash_newfunc,
				      sizeof (struct
					      elf_link_hash_entry),
				      PRU_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->root.hash_table_free = pru_elf32_link_hash_table_free;

  return &ret->root;
}

#define ELF_ARCH			bfd_arch_pru
#define ELF_TARGET_ID			PRU_ELF_DATA
#define ELF_MACHINE_CODE		EM_TI_PRU

#define ELF_MAXPAGESIZE			1

#define bfd_elf32_bfd_link_hash_table_create \
					  pru_elf32_link_hash_table_create

/* Relocation table lookup macros.  */

#define bfd_elf32_bfd_reloc_type_lookup	  pru_elf32_bfd_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	  pru_elf32_bfd_reloc_name_lookup

#define elf_info_to_howto		pru_elf32_info_to_howto
#define elf_info_to_howto_rel		NULL

/* elf backend functions.  */

/* TI folks like to use a mix of REL and RELA relocations.  See also
   the MSP430 and TI C6X backends.  */
#define elf_backend_may_use_rel_p  1
#define elf_backend_may_use_rela_p 1
#define elf_backend_default_use_rela_p 1

#define elf_backend_rela_normal		1

#define elf_backend_relocate_section	pru_elf32_relocate_section
#define bfd_elf32_bfd_relax_section	pru_elf32_relax_section
#define elf_backend_can_gc_sections	1

#define elf_backend_default_execstack	0

#define TARGET_LITTLE_SYM		pru_elf32_vec
#define TARGET_LITTLE_NAME		"elf32-pru"

#include "elf32-target.h"
