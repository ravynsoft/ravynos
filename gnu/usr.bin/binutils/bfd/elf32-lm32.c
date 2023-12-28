/* Lattice Mico32-specific support for 32-bit ELF
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Contributed by Jon Beniston <jon@beniston.com>

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
#include "elf/lm32.h"

#define DEFAULT_STACK_SIZE 0x20000

#define PLT_ENTRY_SIZE 20

#define PLT0_ENTRY_WORD0  0
#define PLT0_ENTRY_WORD1  0
#define PLT0_ENTRY_WORD2  0
#define PLT0_ENTRY_WORD3  0
#define PLT0_ENTRY_WORD4  0

#define PLT0_PIC_ENTRY_WORD0  0
#define PLT0_PIC_ENTRY_WORD1  0
#define PLT0_PIC_ENTRY_WORD2  0
#define PLT0_PIC_ENTRY_WORD3  0
#define PLT0_PIC_ENTRY_WORD4  0

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/libc.so.1"

extern const bfd_target lm32_elf32_fdpic_vec;

#define IS_FDPIC(bfd) ((bfd)->xvec == &lm32_elf32_fdpic_vec)

static bfd_reloc_status_type lm32_elf_gprel_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

/* lm32 ELF linker hash table.  */

struct elf_lm32_link_hash_table
{
  struct elf_link_hash_table root;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sfixup32;
  asection *sdynbss;
  asection *srelbss;

  int relocs32;
};

/* Get the lm32 ELF linker hash table from a link_info structure.  */

#define lm32_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == LM32_ELF_DATA)		\
   ? (struct elf_lm32_link_hash_table *) (p)->hash : NULL)

#define lm32fdpic_got_section(info) \
  (lm32_elf_hash_table (info)->root.sgot)
#define lm32fdpic_gotrel_section(info) \
  (lm32_elf_hash_table (info)->root.srelgot)
#define lm32fdpic_fixup32_section(info) \
  (lm32_elf_hash_table (info)->sfixup32)

struct weak_symbol_list
{
  const char *name;
  struct weak_symbol_list *next;
};

/* Create an lm32 ELF linker hash table.  */

static struct bfd_link_hash_table *
lm32_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_lm32_link_hash_table *ret;
  size_t amt = sizeof (struct elf_lm32_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      _bfd_elf_link_hash_newfunc,
				      sizeof (struct elf_link_hash_entry),
				      LM32_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root.root;
}

/* Add a fixup to the ROFIXUP section.  */

static bfd_vma
_lm32fdpic_add_rofixup (bfd *output_bfd, asection *rofixup, bfd_vma relocation)
{
  bfd_vma fixup_offset;

  if (rofixup->flags & SEC_EXCLUDE)
    return -1;

  fixup_offset = rofixup->reloc_count * 4;
  if (rofixup->contents)
    {
      BFD_ASSERT (fixup_offset < rofixup->size);
      if (fixup_offset < rofixup->size)
      bfd_put_32 (output_bfd, relocation, rofixup->contents + fixup_offset);
    }
  rofixup->reloc_count++;

  return fixup_offset;
}

/* Create .rofixup sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bool
create_rofixup_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf_lm32_link_hash_table *htab;
  htab = lm32_elf_hash_table (info);

  if (htab == NULL)
    return false;

  /* Fixup section for R_LM32_32 relocs.  */
  lm32fdpic_fixup32_section (info)
    = bfd_make_section_anyway_with_flags (dynobj,
					  ".rofixup",
					  (SEC_ALLOC
					   | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_LINKER_CREATED
					   | SEC_READONLY));
  if (lm32fdpic_fixup32_section (info) == NULL
      || !bfd_set_section_alignment (lm32fdpic_fixup32_section (info), 2))
    return false;

  return true;
}

static reloc_howto_type lm32_elf_howto_table [] =
{
  /* This reloc does nothing.  */
  HOWTO (R_LM32_NONE,		    /* type */
	 0,			    /* rightshift */
	 0,			    /* size */
	 0,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_NONE",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0,			    /* dst_mask */
	 false),		    /* pcrel_offset */

  /* An 8 bit absolute relocation.  */
  HOWTO (R_LM32_8,		    /* type */
	 0,			    /* rightshift */
	 1,			    /* size */
	 8,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_8",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xff,			    /* dst_mask */
	 false),		    /* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_LM32_16,		    /* type */
	 0,			    /* rightshift */
	 2,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_16",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  /* A 32 bit absolute relocation.  */
  HOWTO (R_LM32_32,		    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 32,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_32",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffffffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_HI16,		    /* type */
	 16,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_HI16",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_LO16,		    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_LO16",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_GPREL16,	    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 lm32_elf_gprel_reloc,	    /* special_function */
	 "R_LM32_GPREL16",	    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_CALL,		    /* type */
	 2,			    /* rightshift */
	 4,			    /* size */
	 26,			    /* bitsize */
	 true,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_signed,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_CALL",		    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0x3ffffff,		    /* dst_mask */
	 true),			    /* pcrel_offset */

  HOWTO (R_LM32_BRANCH,		    /* type */
	 2,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 true,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_signed,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_BRANCH",	    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 true),			    /* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_LM32_GNU_VTINHERIT,	    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 0,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 NULL,			    /* special_function */
	 "R_LM32_GNU_VTINHERIT",    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0,			    /* dst_mask */
	 false),		    /* pcrel_offset */

  /* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_LM32_GNU_VTENTRY,	    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 0,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn,/* special_function */
	 "R_LM32_GNU_VTENTRY",	    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0,			    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_16_GOT,		    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_signed,  /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_16_GOT",	    /* name */
	 false,			    /* partial_inplace */
	 0,			    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_GOTOFF_HI16,	    /* type */
	 16,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_GOTOFF_HI16",	    /* name */
	 false,			    /* partial_inplace */
	 0xffff,		    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_GOTOFF_LO16,	    /* type */
	 0,			    /* rightshift */
	 4,			    /* size */
	 16,			    /* bitsize */
	 false,			    /* pc_relative */
	 0,			    /* bitpos */
	 complain_overflow_dont,    /* complain_on_overflow */
	 bfd_elf_generic_reloc,	    /* special_function */
	 "R_LM32_GOTOFF_LO16",	    /* name */
	 false,			    /* partial_inplace */
	 0xffff,		    /* src_mask */
	 0xffff,		    /* dst_mask */
	 false),		    /* pcrel_offset */

  HOWTO (R_LM32_COPY,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LM32_COPY",		/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_LM32_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LM32_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_LM32_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LM32_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_LM32_RELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LM32_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

};

/* Map BFD reloc types to lm32 ELF reloc types. */

struct lm32_reloc_map
{
    bfd_reloc_code_real_type bfd_reloc_val;
    unsigned char elf_reloc_val;
};

static const struct lm32_reloc_map lm32_reloc_map[] =
{
  { BFD_RELOC_NONE,		R_LM32_NONE },
  { BFD_RELOC_8,		R_LM32_8 },
  { BFD_RELOC_16,		R_LM32_16 },
  { BFD_RELOC_32,		R_LM32_32 },
  { BFD_RELOC_HI16,		R_LM32_HI16 },
  { BFD_RELOC_LO16,		R_LM32_LO16 },
  { BFD_RELOC_GPREL16,		R_LM32_GPREL16 },
  { BFD_RELOC_LM32_CALL,	R_LM32_CALL },
  { BFD_RELOC_LM32_BRANCH,	R_LM32_BRANCH },
  { BFD_RELOC_VTABLE_INHERIT,	R_LM32_GNU_VTINHERIT },
  { BFD_RELOC_VTABLE_ENTRY,	R_LM32_GNU_VTENTRY },
  { BFD_RELOC_LM32_16_GOT,	R_LM32_16_GOT },
  { BFD_RELOC_LM32_GOTOFF_HI16, R_LM32_GOTOFF_HI16 },
  { BFD_RELOC_LM32_GOTOFF_LO16, R_LM32_GOTOFF_LO16 },
  { BFD_RELOC_LM32_COPY,	R_LM32_COPY },
  { BFD_RELOC_LM32_GLOB_DAT,	R_LM32_GLOB_DAT },
  { BFD_RELOC_LM32_JMP_SLOT,	R_LM32_JMP_SLOT },
  { BFD_RELOC_LM32_RELATIVE,	R_LM32_RELATIVE },
};

static reloc_howto_type *
lm32_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (lm32_reloc_map) / sizeof (lm32_reloc_map[0]); i++)
    if (lm32_reloc_map[i].bfd_reloc_val == code)
      return &lm32_elf_howto_table[lm32_reloc_map[i].elf_reloc_val];
  return NULL;
}

static reloc_howto_type *
lm32_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (lm32_elf_howto_table) / sizeof (lm32_elf_howto_table[0]);
       i++)
    if (lm32_elf_howto_table[i].name != NULL
	&& strcasecmp (lm32_elf_howto_table[i].name, r_name) == 0)
      return &lm32_elf_howto_table[i];

  return NULL;
}


/* Set the howto pointer for an Lattice Mico32 ELF reloc.  */

static bool
lm32_info_to_howto_rela (bfd *abfd,
			 arelent *cache_ptr,
			 Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_LM32_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &lm32_elf_howto_table[r_type];
  return true;
}

/* Set the right machine number for an Lattice Mico32 ELF file. */

static bool
lm32_elf_object_p (bfd *abfd)
{
  return bfd_default_set_arch_mach (abfd, bfd_arch_lm32, bfd_mach_lm32);
}

/* Set machine type flags just before file is written out. */

static bool
lm32_elf_final_write_processing (bfd *abfd)
{
  elf_elfheader (abfd)->e_machine = EM_LATTICEMICO32;
  elf_elfheader (abfd)->e_flags &=~ EF_LM32_MACH;
  switch (bfd_get_mach (abfd))
    {
      case bfd_mach_lm32:
	elf_elfheader (abfd)->e_flags |= E_LM32_MACH;
	break;
      default:
	abort ();
    }
  return _bfd_elf_final_write_processing (abfd);
}

/* Set the GP value for OUTPUT_BFD.  Returns FALSE if this is a
   dangerous relocation.  */

static bool
lm32_elf_assign_gp (bfd *output_bfd, bfd_vma *pgp)
{
  unsigned int count;
  asymbol **sym;
  unsigned int i;

  /* If we've already figured out what GP will be, just return it. */
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
	  const char *name;

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
lm32_elf_final_gp (bfd *output_bfd, asymbol *symbol, bool relocatable,
		    char **error_message, bfd_vma *pgp)
{
  if (bfd_is_und_section (symbol->section) && !relocatable)
    {
      *pgp = 0;
      return bfd_reloc_undefined;
    }

  *pgp = _bfd_get_gp_value (output_bfd);
  if (*pgp == 0 && (!relocatable || (symbol->flags & BSF_SECTION_SYM) != 0))
    {
      if (relocatable)
	{
	  /* Make up a value.  */
	  *pgp = symbol->section->output_section->vma + 0x4000;
	  _bfd_set_gp_value (output_bfd, *pgp);
	}
      else if (!lm32_elf_assign_gp (output_bfd, pgp))
	{
	  *error_message =
	    (char *)
	    _("global pointer relative relocation when _gp not defined");
	  return bfd_reloc_dangerous;
	}
    }

  return bfd_reloc_ok;
}

static bfd_reloc_status_type
lm32_elf_do_gprel_relocate (bfd *abfd,
			    reloc_howto_type *howto,
			    asection *input_section ATTRIBUTE_UNUSED,
			    bfd_byte *data,
			    bfd_vma offset,
			    bfd_vma symbol_value,
			    bfd_vma addend)
{
  return _bfd_final_link_relocate (howto, abfd, input_section,
				   data, offset, symbol_value, addend);
}

static bfd_reloc_status_type
lm32_elf_gprel_reloc (bfd *abfd,
		      arelent *reloc_entry,
		      asymbol *symbol,
		      void *data,
		      asection *input_section,
		      bfd *output_bfd,
		      char **msg)
{
  bfd_vma relocation;
  bfd_vma gp;
  bfd_reloc_status_type r;

  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (!reloc_entry->howto->partial_inplace || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    return bfd_reloc_ok;

  relocation = symbol->value
    + symbol->section->output_section->vma + symbol->section->output_offset;

  if ((r =
       lm32_elf_final_gp (abfd, symbol, false, msg, &gp)) == bfd_reloc_ok)
    {
      relocation = relocation + reloc_entry->addend - gp;
      reloc_entry->addend = 0;
      if ((signed) relocation < -32768 || (signed) relocation > 32767)
	{
	  *msg = _("global pointer relative address out of range");
	  r = bfd_reloc_outofrange;
	}
      else
	{
	  r = lm32_elf_do_gprel_relocate (abfd, reloc_entry->howto,
					     input_section,
					     data, reloc_entry->address,
					     relocation, reloc_entry->addend);
	}
    }

  return r;
}

/* Find the segment number in which OSEC, and output section, is
   located.  */

static unsigned
_lm32fdpic_osec_to_segment (bfd *output_bfd, asection *osec)
{
  struct elf_segment_map *m;
  Elf_Internal_Phdr *p;

  /* Find the segment that contains the output_section.  */
  for (m = elf_seg_map (output_bfd), p = elf_tdata (output_bfd)->phdr;
       m != NULL;
       m = m->next, p++)
    {
      int i;

      for (i = m->count - 1; i >= 0; i--)
	if (m->sections[i] == osec)
	  break;

      if (i >= 0)
	break;
    }

  return p - elf_tdata (output_bfd)->phdr;
}

/* Determine if an output section is read-only.  */

inline static bool
_lm32fdpic_osec_readonly_p (bfd *output_bfd, asection *osec)
{
  unsigned seg = _lm32fdpic_osec_to_segment (output_bfd, osec);

  return ! (elf_tdata (output_bfd)->phdr[seg].p_flags & PF_W);
}

/* Relocate a section */

static int
lm32_elf_relocate_section (bfd *output_bfd,
			   struct bfd_link_info *info,
			   bfd *input_bfd,
			   asection *input_section,
			   bfd_byte *contents,
			   Elf_Internal_Rela *relocs,
			   Elf_Internal_Sym *local_syms,
			   asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  Elf_Internal_Rela *rel, *relend;
  struct elf_lm32_link_hash_table *htab = lm32_elf_hash_table (info);
  bfd_vma *local_got_offsets;
  asection *sgot;

  if (htab == NULL)
    return false;

  local_got_offsets = elf_local_got_offsets (input_bfd);

  sgot = htab->root.sgot;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned int r_type;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_vma gp;
      bfd_reloc_status_type r;
      const char *name = NULL;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type == R_LM32_GNU_VTENTRY
	  || r_type == R_LM32_GNU_VTINHERIT )
	continue;

      h = NULL;
      sym = NULL;
      sec = NULL;

      howto = lm32_elf_howto_table + r_type;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* It's a local symbol.  */
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  /* It's a global symbol.  */
	  bool unresolved_reloc;
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	{
	  /* This is a relocatable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (sym == NULL || ELF_ST_TYPE (sym->st_info) != STT_SECTION)
	    continue;

	  /* If partial_inplace, we need to store any additional addend
	     back in the section.  */
	  if (! howto->partial_inplace)
	    continue;

	  /* Shouldn't reach here.  */
	  abort ();
	  r = bfd_reloc_ok;
	}
      else
	{
	  switch (howto->type)
	    {
	    case R_LM32_GPREL16:
	      if (!lm32_elf_assign_gp (output_bfd, &gp))
		r = bfd_reloc_dangerous;
	      else
		{
		  relocation = relocation + rel->r_addend - gp;
		  rel->r_addend = 0;
		  if ((signed)relocation < -32768 || (signed)relocation > 32767)
		    r = bfd_reloc_outofrange;
		  else
		    {
		      r = _bfd_final_link_relocate (howto, input_bfd,
						    input_section, contents,
						    rel->r_offset, relocation,
						    rel->r_addend);
		    }
		}
	      break;
	    case R_LM32_16_GOT:
	      /* Relocation is to the entry for this symbol in the global
		 offset table.  */
	      BFD_ASSERT (sgot != NULL);
	      if (h != NULL)
		{
		  bool dyn;
		  bfd_vma off;

		  off = h->got.offset;
		  BFD_ASSERT (off != (bfd_vma) -1);

		  dyn = htab->root.dynamic_sections_created;
		  if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							 bfd_link_pic (info),
							 h)
		      || (bfd_link_pic (info)
			  && (info->symbolic
			      || h->dynindx == -1
			      || h->forced_local)
			  && h->def_regular))
		    {
		      /* This is actually a static link, or it is a
			 -Bsymbolic link and the symbol is defined
			 locally, or the symbol was forced to be local
			 because of a version file.  We must initialize
			 this entry in the global offset table.  Since the
			 offset must always be a multiple of 4, we use the
			 least significant bit to record whether we have
			 initialized it already.

			 When doing a dynamic link, we create a .rela.got
			 relocation entry to initialize the value.  This
			 is done in the finish_dynamic_symbol routine.  */
		      if ((off & 1) != 0)
			off &= ~1;
		      else
			{
			  /* Write entry in GOT */
			  bfd_put_32 (output_bfd, relocation,
				      sgot->contents + off);
			  /* Create entry in .rofixup pointing to GOT entry.  */
			   if (IS_FDPIC (output_bfd) && h->root.type != bfd_link_hash_undefweak)
			     {
			       _lm32fdpic_add_rofixup (output_bfd,
						       lm32fdpic_fixup32_section
							(info),
						       sgot->output_section->vma
							+ sgot->output_offset
							+ off);
			     }
			  /* Mark GOT entry as having been written.  */
			  h->got.offset |= 1;
			}
		    }

		  relocation = sgot->output_offset + off;
		}
	      else
		{
		  bfd_vma off;
		  bfd_byte *loc;

		  BFD_ASSERT (local_got_offsets != NULL
			      && local_got_offsets[r_symndx] != (bfd_vma) -1);

		  /* Get offset into GOT table.  */
		  off = local_got_offsets[r_symndx];

		  /* The offset must always be a multiple of 4.  We use
		     the least significant bit to record whether we have
		     already processed this entry.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      /* Write entry in GOT.  */
		      bfd_put_32 (output_bfd, relocation, sgot->contents + off);
		      /* Create entry in .rofixup pointing to GOT entry.  */
		      if (IS_FDPIC (output_bfd))
			{
			  _lm32fdpic_add_rofixup (output_bfd,
						  lm32fdpic_fixup32_section
						   (info),
						  sgot->output_section->vma
						   + sgot->output_offset
						   + off);
			}

		      if (bfd_link_pic (info))
			{
			  asection *srelgot;
			  Elf_Internal_Rela outrel;

			  /* We need to generate a R_LM32_RELATIVE reloc
			     for the dynamic linker.  */
			  srelgot = htab->root.srelgot;
			  BFD_ASSERT (srelgot != NULL);

			  outrel.r_offset = (sgot->output_section->vma
					     + sgot->output_offset
					     + off);
			  outrel.r_info = ELF32_R_INFO (0, R_LM32_RELATIVE);
			  outrel.r_addend = relocation;
			  loc = srelgot->contents;
			  loc += srelgot->reloc_count * sizeof (Elf32_External_Rela);
			  bfd_elf32_swap_reloca_out (output_bfd, &outrel,loc);
			  ++srelgot->reloc_count;
			}

		      local_got_offsets[r_symndx] |= 1;
		    }


		  relocation = sgot->output_offset + off;
		}

	      /* Addend should be zero.  */
	      if (rel->r_addend != 0)
		_bfd_error_handler
		  (_("internal error: addend should be zero for %s"),
		   "R_LM32_16_GOT");

	      r = _bfd_final_link_relocate (howto,
					    input_bfd,
					    input_section,
					    contents,
					    rel->r_offset,
					    relocation,
					    rel->r_addend);
	      break;

	    case R_LM32_GOTOFF_LO16:
	    case R_LM32_GOTOFF_HI16:
	      /* Relocation is offset from GOT.  */
	      BFD_ASSERT (sgot != NULL);
	      relocation -= sgot->output_section->vma;
	      /* Account for sign-extension.  */
	      if ((r_type == R_LM32_GOTOFF_HI16)
		  && ((relocation + rel->r_addend) & 0x8000))
		rel->r_addend += 0x10000;
	      r = _bfd_final_link_relocate (howto,
					    input_bfd,
					    input_section,
					    contents,
					    rel->r_offset,
					    relocation,
					    rel->r_addend);
	      break;

	    case R_LM32_32:
	      if (IS_FDPIC (output_bfd))
		{
		  if ((!h) || (h && h->root.type != bfd_link_hash_undefweak))
		    {
		      /* Only create .rofixup entries for relocs in loadable sections.  */
		      if ((bfd_section_flags (input_section->output_section)
			  & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))

			{
			  /* Check address to be modified is writable.  */
			  if (_lm32fdpic_osec_readonly_p (output_bfd,
							  input_section
							   ->output_section))
			    {
			      info->callbacks->warning
				(info,
				 _("cannot emit dynamic relocations in read-only section"),
				 name, input_bfd, input_section, rel->r_offset);
			       return false;
			    }
			  /* Create entry in .rofixup section.  */
			  _lm32fdpic_add_rofixup (output_bfd,
						  lm32fdpic_fixup32_section (info),
						  input_section->output_section->vma
						   + input_section->output_offset
						   + rel->r_offset);
			}
		    }
		}
	      /* Fall through.  */

	    default:
	      r = _bfd_final_link_relocate (howto,
					    input_bfd,
					    input_section,
					    contents,
					    rel->r_offset,
					    relocation,
					    rel->r_addend);
	      break;
	    }
	}

      if (r != bfd_reloc_ok)
	{
	  const char *msg = NULL;
	  arelent bfd_reloc;

	  if (! lm32_info_to_howto_rela (input_bfd, &bfd_reloc, rel))
	    continue;
	  howto = bfd_reloc.howto;

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
	      if ((h != NULL)
		 && (h->root.type == bfd_link_hash_undefweak))
		break;
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
	      /* fall through */

	    common_error:
	      (*info->callbacks->warning) (info, msg, name, input_bfd,
					   input_section, rel->r_offset);
	      break;
	    }
	}
    }

  return true;
}

static asection *
lm32_elf_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_LM32_GNU_VTINHERIT:
      case R_LM32_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Look through the relocs for a section during the first phase.  */

static bool
lm32_elf_check_relocs (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  struct elf_lm32_link_hash_table *htab;
  bfd *dynobj;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      int r_type;
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);
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
      if (htab->root.sgot == NULL)
	{
	  switch (r_type)
	    {
	    case R_LM32_16_GOT:
	    case R_LM32_GOTOFF_HI16:
	    case R_LM32_GOTOFF_LO16:
	      if (dynobj == NULL)
		htab->root.dynobj = dynobj = abfd;
	      if (!_bfd_elf_create_got_section (dynobj, info))
		return false;
	      break;
	    }
	}

      /* Some relocs require a rofixup table. */
      if (IS_FDPIC (abfd))
	{
	  switch (r_type)
	    {
	    case R_LM32_32:
	      /* FDPIC requires a GOT if there is a .rofixup section
		 (Normal ELF doesn't). */
	      if (dynobj == NULL)
		htab->root.dynobj = dynobj = abfd;
	      if (!_bfd_elf_create_got_section (dynobj, info))
		return false;
	      /* Create .rofixup section */
	      if (htab->sfixup32 == NULL)
		{
		  if (! create_rofixup_section (dynobj, info))
		    return false;
		}
	      break;
	    case R_LM32_16_GOT:
	    case R_LM32_GOTOFF_HI16:
	    case R_LM32_GOTOFF_LO16:
	      /* Create .rofixup section.  */
	      if (htab->sfixup32 == NULL)
		{
		  if (dynobj == NULL)
		    htab->root.dynobj = dynobj = abfd;
		  if (! create_rofixup_section (dynobj, info))
		    return false;
		}
	      break;
	    }
	}

      switch (r_type)
	{
	case R_LM32_16_GOT:
	  if (h != NULL)
	    h->got.refcount += 1;
	  else
	    {
	      bfd_signed_vma *local_got_refcounts;

	      /* This is a global offset table entry for a local symbol.  */
	      local_got_refcounts = elf_local_got_refcounts (abfd);
	      if (local_got_refcounts == NULL)
		{
		  bfd_size_type size;

		  size = symtab_hdr->sh_info;
		  size *= sizeof (bfd_signed_vma);
		  local_got_refcounts = bfd_zalloc (abfd, size);
		  if (local_got_refcounts == NULL)
		    return false;
		  elf_local_got_refcounts (abfd) = local_got_refcounts;
		}
	      local_got_refcounts[r_symndx] += 1;
	    }
	  break;

	/* This relocation describes the C++ object vtable hierarchy.
	   Reconstruct it for later use during GC.  */
	case R_LM32_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	/* This relocation describes which C++ vtable entries are actually
	   used.  Record for later use during GC.  */
	case R_LM32_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	}
    }

  return true;
}

/* Finish up the dynamic sections.  */

static bool
lm32_elf_finish_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct elf_lm32_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;
  asection *sgot;

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;

  sgot = htab->root.sgotplt;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sgot != NULL && sdyn != NULL);

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
	      s = htab->root.sgotplt;
	      goto get_vma;
	    case DT_JMPREL:
	      s = htab->root.srelplt;
	    get_vma:
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      splt = htab->root.splt;
      if (splt && splt->size > 0)
	{
	  if (bfd_link_pic (info))
	    {
	      bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD0, splt->contents);
	      bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD1, splt->contents + 4);
	      bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD2, splt->contents + 8);
	      bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD3, splt->contents + 12);
	      bfd_put_32 (output_bfd, PLT0_PIC_ENTRY_WORD4, splt->contents + 16);
	    }
	  else
	    {
	      unsigned long addr;
	      /* addr = .got + 4 */
	      addr = sgot->output_section->vma + sgot->output_offset + 4;
	      bfd_put_32 (output_bfd,
			  PLT0_ENTRY_WORD0 | ((addr >> 16) & 0xffff),
			  splt->contents);
	      bfd_put_32 (output_bfd,
			  PLT0_ENTRY_WORD1 | (addr & 0xffff),
			  splt->contents + 4);
	      bfd_put_32 (output_bfd, PLT0_ENTRY_WORD2, splt->contents + 8);
	      bfd_put_32 (output_bfd, PLT0_ENTRY_WORD3, splt->contents + 12);
	      bfd_put_32 (output_bfd, PLT0_ENTRY_WORD4, splt->contents + 16);
	    }

	  elf_section_data (splt->output_section)->this_hdr.sh_entsize =
	    PLT_ENTRY_SIZE;
	}
    }

  /* Fill in the first three entries in the global offset table.  */
  if (sgot && sgot->size > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgot->contents);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 8);

      /* FIXME:  This can be null if create_dynamic_sections wasn't called. */
      if (elf_section_data (sgot->output_section) != NULL)
	elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (lm32fdpic_fixup32_section (info))
    {
      struct elf_link_hash_entry *hgot = elf_hash_table (info)->hgot;
      bfd_vma got_value = hgot->root.u.def.value
	    + hgot->root.u.def.section->output_section->vma
	    + hgot->root.u.def.section->output_offset;
      struct bfd_link_hash_entry *hend;

      /* Last entry is pointer to GOT.  */
      _lm32fdpic_add_rofixup (output_bfd, lm32fdpic_fixup32_section (info), got_value);

      /* Check we wrote enough entries.  */
      if (lm32fdpic_fixup32_section (info)->size
	      != (lm32fdpic_fixup32_section (info)->reloc_count * 4))
	{
	  _bfd_error_handler
	    ("LINKER BUG: .rofixup section size mismatch: size/4 %" PRId64
	     " != relocs %d",
	    (int64_t) (lm32fdpic_fixup32_section (info)->size / 4),
	    lm32fdpic_fixup32_section (info)->reloc_count);
	  return false;
	}

      hend = bfd_link_hash_lookup (info->hash, "__ROFIXUP_END__",
	      false, false, true);
      if (hend
	  && (hend->type == bfd_link_hash_defined
	      || hend->type == bfd_link_hash_defweak)
	  && hend->u.def.section->output_section != NULL)
	{
	  bfd_vma value =
	    lm32fdpic_fixup32_section (info)->output_section->vma
	    + lm32fdpic_fixup32_section (info)->output_offset
	    + lm32fdpic_fixup32_section (info)->size
	    - hend->u.def.section->output_section->vma
	    - hend->u.def.section->output_offset;
	  BFD_ASSERT (hend->u.def.value == value);
	  if (hend->u.def.value != value)
	    {
	      _bfd_error_handler
		("LINKER BUG: .rofixup section hend->u.def.value != value: %"
		 PRId64 " != %" PRId64,
		 (int64_t) hend->u.def.value, (int64_t) value);
	      return false;
	    }
	}
    }

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
lm32_elf_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  struct elf_lm32_link_hash_table *htab;
  bfd_byte *loc;

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgot;
      asection *srela;

      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = htab->root.splt;
      sgot = htab->root.sgotplt;
      srela = htab->root.srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

      /* Get the offset into the .got table of the entry that
	corresponds to this function.  Each .got entry is 4 bytes.
	The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;

      /* Fill in the entry in the procedure linkage table.  */
      if (! bfd_link_pic (info))
	{
	  /* TODO */
	}
      else
	{
	  /* TODO */
	}

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset
		   + h->plt.offset
		   + 12), /* same offset */
		  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_LM32_JMP_SLOT);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}

    }

  if (h->got.offset != (bfd_vma) -1)
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      sgot = htab->root.sgot;
      srela = htab->root.srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + (h->got.offset &~ 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
	 locally, we just want to emit a RELATIVE reloc.  Likewise if
	 the symbol was forced to be local because of a version file.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info)
	  && (info->symbolic
	      || h->dynindx == -1
	      || h->forced_local)
	  && h->def_regular)
	{
	  rela.r_info = ELF32_R_INFO (0, R_LM32_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_LM32_GLOB_DAT);
	  rela.r_addend = 0;
	}

      loc = srela->contents;
      loc += srela->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++srela->reloc_count;
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rela;

      /* This symbols needs a copy reloc.  Set it up.  */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_linker_section (htab->root.dynobj, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_LM32_COPY);
      rela.r_addend = 0;
      loc = s->contents;
      loc += s->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
      ++s->reloc_count;
    }

  /* Mark some specially defined symbols as absolute.  */
  if (h == htab->root.hdynamic || h == htab->root.hgot)
    sym->st_shndx = SHN_ABS;

  return true;
}

static enum elf_reloc_type_class
lm32_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_LM32_RELATIVE:  return reloc_class_relative;
    case R_LM32_JMP_SLOT:  return reloc_class_plt;
    case R_LM32_COPY:      return reloc_class_copy;
    default:		   return reloc_class_normal;
    }
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
lm32_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h)
{
  struct elf_lm32_link_hash_table *htab;
  bfd *dynobj;
  asection *s;

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
      if (! bfd_link_pic (info)
	  && !h->def_dynamic
	  && !h->ref_dynamic
	  && h->root.type != bfd_link_hash_undefweak
	  && h->root.type != bfd_link_hash_undefined)
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a PCREL
	     reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
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
  if (0 && info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (0 && !_bfd_elf_readonly_dynrelocs (h))
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

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  s = htab->sdynbss;
  BFD_ASSERT (s != NULL);

  /* We must generate a R_LM32_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      srel = htab->srelbss;
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf_lm32_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (htab->root.dynamic_sections_created
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
	  asection *s = htab->root.splt;

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
	  htab->root.sgotplt->size += 4;

	  /* We also need to make an entry in the .rel.plt section.  */
	  htab->root.srelplt->size += sizeof (Elf32_External_Rela);
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
      bool dyn;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->root.sgot;

      h->got.offset = s->size;
      s->size += 4;
      dyn = htab->root.dynamic_sections_created;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h))
	htab->root.srelgot->size += sizeof (Elf32_External_Rela);
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

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL;)
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
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
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->root.dynamic_sections_created
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
lm32_elf_size_dynamic_sections (bfd *output_bfd,
				struct bfd_link_info *info)
{
  struct elf_lm32_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
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

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
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
	      if (! bfd_is_abs_section (p->sec)
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
      s = htab->root.sgot;
      srel = htab->root.srelgot;
      for (; local_got < end_local_got; ++local_got)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += 4;
	      if (bfd_link_pic (info))
		srel->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt
	  || s == htab->root.sgot
	  || s == htab->root.sgotplt
	  || s == htab->sdynbss)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0 && s != htab->root.srelplt)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
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
	 but this way if it does, we get a R_LM32_NONE reloc instead
	 of garbage.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  if (!_bfd_elf_add_dynamic_tags (output_bfd, info, relocs))
    return false;

  /* Allocate .rofixup section.  */
  if (IS_FDPIC (output_bfd))
    {
      struct weak_symbol_list *list_start = NULL, *list_end = NULL;
      int rgot_weak_count = 0;
      int r32_count = 0;
      int rgot_count ATTRIBUTE_UNUSED = 0;
      /* Look for deleted sections.  */
      for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
	{
	  for (s = ibfd->sections; s != NULL; s = s->next)
	    {
	      if (s->reloc_count)
		{
		  /* Count relocs that need .rofixup entires.  */
		  Elf_Internal_Rela *internal_relocs, *end;
		  internal_relocs = elf_section_data (s)->relocs;
		  if (internal_relocs == NULL)
		    internal_relocs = (_bfd_elf_link_read_relocs (ibfd, s, NULL, NULL, false));
		  if (internal_relocs != NULL)
		    {
		      end = internal_relocs + s->reloc_count;
		      while (internal_relocs < end)
			{
			  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
			  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (ibfd);
			  unsigned long r_symndx;
			  struct elf_link_hash_entry *h;

			  symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
			  sym_hashes = elf_sym_hashes (ibfd);
			  r_symndx = ELF32_R_SYM (internal_relocs->r_info);
			  h = NULL;
			  if (r_symndx < symtab_hdr->sh_info)
			    {
			    }
			  else
			    {
			      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
			      while (h->root.type == bfd_link_hash_indirect
				     || h->root.type == bfd_link_hash_warning)
				h = (struct elf_link_hash_entry *) h->root.u.i.link;
			      }

			  /* Don't generate entries for weak symbols.  */
			  if (!h || (h && h->root.type != bfd_link_hash_undefweak))
			    {
			      if (!discarded_section (s) && !((bfd_section_flags (s) & SEC_ALLOC) == 0))
				{
				  switch (ELF32_R_TYPE (internal_relocs->r_info))
				    {
				    case R_LM32_32:
				      r32_count++;
				      break;
				    case R_LM32_16_GOT:
				      rgot_count++;
				      break;
				    }
				}
			    }
			  else
			    {
			      struct weak_symbol_list *current, *new_entry;
			      /* Is this symbol already in the list?  */
			      for (current = list_start; current; current = current->next)
				{
				  if (!strcmp (current->name, h->root.root.string))
				    break;
				}
			      if (!current && !discarded_section (s) && (bfd_section_flags (s) & SEC_ALLOC))
				{
				  /* Will this have an entry in the GOT.  */
				  if (ELF32_R_TYPE (internal_relocs->r_info) == R_LM32_16_GOT)
				    {
				      /* Create a new entry.  */
				      new_entry = malloc (sizeof (struct weak_symbol_list));
				      if (!new_entry)
					return false;
				      new_entry->name = h->root.root.string;
				      new_entry->next = NULL;
				      /* Add to list */
				      if (list_start == NULL)
					{
					  list_start = new_entry;
					  list_end = new_entry;
					}
				      else
					{
					  list_end->next = new_entry;
					  list_end = new_entry;
					}
				      /* Increase count of undefined weak symbols in the got.  */
				      rgot_weak_count++;
				    }
				}
			    }
			  internal_relocs++;
			}
		    }
		  else
		    return false;
		}
	    }
	}
      /* Free list.  */
      while (list_start)
	{
	  list_end = list_start->next;
	  free (list_start);
	  list_start = list_end;
	}

      /* Size sections.  */
      lm32fdpic_fixup32_section (info)->size
	= (r32_count + (htab->root.sgot->size / 4) - rgot_weak_count + 1) * 4;
      if (lm32fdpic_fixup32_section (info)->size == 0)
	lm32fdpic_fixup32_section (info)->flags |= SEC_EXCLUDE;
      else
	{
	  lm32fdpic_fixup32_section (info)->contents =
	     bfd_zalloc (dynobj, lm32fdpic_fixup32_section (info)->size);
	  if (lm32fdpic_fixup32_section (info)->contents == NULL)
	    return false;
	}
    }

  return true;
}

/* Create dynamic sections when linking against a dynamic object.  */

static bool
lm32_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_lm32_link_hash_table *htab;
  flagword flags, pltflags;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ptralign = 2; /* 32bit */

  htab = lm32_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Make sure we have a GOT - For the case where we have a dynamic object
     but none of the relocs in check_relocs */
  if (!_bfd_elf_create_got_section (abfd, info))
    return false;
  if (IS_FDPIC (abfd) && (htab->sfixup32 == NULL))
    {
      if (! create_rofixup_section (abfd, info))
	return false;
    }

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */
  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  pltflags = flags;
  pltflags |= SEC_CODE;
  if (bed->plt_not_loaded)
    pltflags &= ~ (SEC_LOAD | SEC_HAS_CONTENTS);
  if (bed->plt_readonly)
    pltflags |= SEC_READONLY;

  s = bfd_make_section_anyway_with_flags (abfd, ".plt", pltflags);
  htab->root.splt = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->plt_alignment))
    return false;

  if (bed->want_plt_sym)
    {
      /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
	 .plt section.  */
      struct bfd_link_hash_entry *bh = NULL;
      struct elf_link_hash_entry *h;

      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, "_PROCEDURE_LINKAGE_TABLE_", BSF_GLOBAL, s,
	      (bfd_vma) 0, NULL, false,
	      get_elf_backend_data (abfd)->collect, &bh)))
	return false;
      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;
      htab->root.hplt = h;

      if (bfd_link_pic (info)
	  && ! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  s = bfd_make_section_anyway_with_flags (abfd,
					  bed->default_use_rela_p
					  ? ".rela.plt" : ".rel.plt",
					  flags | SEC_READONLY);
  htab->root.srelplt = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, ptralign))
    return false;

  if (htab->root.sgot == NULL
      && !_bfd_elf_create_got_section (abfd, info))
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
      htab->sdynbss = s;
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
      if (! bfd_link_pic (info))
	{
	  s = bfd_make_section_anyway_with_flags (abfd,
						  (bed->default_use_rela_p
						   ? ".rela.bss" : ".rel.bss"),
						  flags | SEC_READONLY);
	  htab->srelbss = s;
	  if (s == NULL
	      || !bfd_set_section_alignment (s, ptralign))
	    return false;
	}
    }

  return true;
}

static bool
lm32_elf_always_size_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  if (!bfd_link_relocatable (info))
    {
      if (!bfd_elf_stack_segment_size (output_bfd, info,
				       "__stacksize", DEFAULT_STACK_SIZE))
	return false;

      asection *sec = bfd_get_section_by_name (output_bfd, ".stack");
      if (sec)
	sec->size = info->stacksize >= 0 ? info->stacksize : 0;
    }

  return true;
}

static bool
lm32_elf_fdpic_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  unsigned i;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  if (! _bfd_elf_copy_private_bfd_data (ibfd, obfd))
    return false;

  if (! elf_tdata (ibfd) || ! elf_tdata (ibfd)->phdr
      || ! elf_tdata (obfd) || ! elf_tdata (obfd)->phdr)
    return true;

  /* Copy the stack size.  */
  for (i = 0; i < elf_elfheader (ibfd)->e_phnum; i++)
    if (elf_tdata (ibfd)->phdr[i].p_type == PT_GNU_STACK)
      {
	Elf_Internal_Phdr *iphdr = &elf_tdata (ibfd)->phdr[i];

	for (i = 0; i < elf_elfheader (obfd)->e_phnum; i++)
	  if (elf_tdata (obfd)->phdr[i].p_type == PT_GNU_STACK)
	    {
	      memcpy (&elf_tdata (obfd)->phdr[i], iphdr, sizeof (*iphdr));

	      /* Rewrite the phdrs, since we're only called after they were first written.  */
	      if (bfd_seek (obfd, (bfd_signed_vma) get_elf_backend_data (obfd)
			    ->s->sizeof_ehdr, SEEK_SET) != 0
		  || get_elf_backend_data (obfd)->s->write_out_phdrs (obfd, elf_tdata (obfd)->phdr,
				     elf_elfheader (obfd)->e_phnum) != 0)
		return false;
	      break;
	    }

	break;
      }

  return true;
}


#define ELF_ARCH		bfd_arch_lm32
#define ELF_TARGET_ID		LM32_ELF_DATA
#define ELF_MACHINE_CODE	EM_LATTICEMICO32
#define ELF_MAXPAGESIZE		0x1000

#define TARGET_BIG_SYM		lm32_elf32_vec
#define TARGET_BIG_NAME		"elf32-lm32"

#define bfd_elf32_bfd_reloc_type_lookup		lm32_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		lm32_reloc_name_lookup
#define elf_info_to_howto			lm32_info_to_howto_rela
#define elf_info_to_howto_rel			NULL
#define elf_backend_rela_normal			1
#define elf_backend_object_p			lm32_elf_object_p
#define elf_backend_final_write_processing	lm32_elf_final_write_processing
#define elf_backend_stack_align			8
#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_gc_mark_hook		lm32_elf_gc_mark_hook
#define elf_backend_plt_readonly		1
#define elf_backend_want_got_plt		1
#define elf_backend_want_plt_sym		0
#define elf_backend_got_header_size		12
#define elf_backend_dtrel_excludes_plt		1
#define bfd_elf32_bfd_link_hash_table_create	lm32_elf_link_hash_table_create
#define elf_backend_check_relocs		lm32_elf_check_relocs
#define elf_backend_reloc_type_class		lm32_elf_reloc_type_class
#define elf_backend_size_dynamic_sections	lm32_elf_size_dynamic_sections
#define elf_backend_omit_section_dynsym		_bfd_elf_omit_section_dynsym_all
#define elf_backend_create_dynamic_sections	lm32_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections	lm32_elf_finish_dynamic_sections
#define elf_backend_adjust_dynamic_symbol	lm32_elf_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol	lm32_elf_finish_dynamic_symbol
#define elf_backend_relocate_section		lm32_elf_relocate_section

#include "elf32-target.h"

#undef  ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE		0x4000


#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM		lm32_elf32_fdpic_vec
#undef	TARGET_BIG_NAME
#define TARGET_BIG_NAME		"elf32-lm32fdpic"
#undef	elf32_bed
#define	elf32_bed		elf32_lm32fdpic_bed

#undef	elf_backend_always_size_sections
#define elf_backend_always_size_sections	lm32_elf_always_size_sections
#undef	bfd_elf32_bfd_copy_private_bfd_data
#define bfd_elf32_bfd_copy_private_bfd_data	lm32_elf_fdpic_copy_private_bfd_data

#include "elf32-target.h"
