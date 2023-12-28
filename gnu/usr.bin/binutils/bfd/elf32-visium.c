/* Visium-specific support for 32-bit ELF.

   Copyright (C) 2003-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/visium.h"
#include "libiberty.h"

static bfd_reloc_status_type visium_elf_howto_parity_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);

static reloc_howto_type visium_elf_howto_table[] = {
  /* This reloc does nothing.  */
  HOWTO (R_VISIUM_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 8 bit absolute relocation.  */
  HOWTO (R_VISIUM_8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_8",		/* name */
	 false,			/* partial_inplace */
	 0x00,			/* src_mask */
	 0xff,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_VISIUM_16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_16",		/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 32 bit absolute relocation.  */
  HOWTO (R_VISIUM_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_32",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */


  /* A 8 bit PC relative relocation.  */
  HOWTO (R_VISIUM_8_PCREL,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_8_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x00,			/* src_mask */
	 0xff,			/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit PC relative relocation.  */
  HOWTO (R_VISIUM_16_PCREL,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_16_PCREL",	/* name */
	 false,			/* partial inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 32-bit PC relative relocation.  */
  HOWTO (R_VISIUM_32_PCREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VISIUM_32_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16-bit PC word relative offset, relative to start of instruction
     and always in the second half of the instruction.  */
  HOWTO (R_VISIUM_PC16,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_PC16",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* The high 16 bits of symbol value.  */
  HOWTO (R_VISIUM_HI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The low 16 bits of symbol value.  */
  HOWTO (R_VISIUM_LO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 16 bit immediate value.  */
  HOWTO (R_VISIUM_IM16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_IM16",	/* name */
	 false,			/* partial_inplace */
	 0x0000000,		/* src_mask */
	 0x000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* The high 16 bits of symbol value, pc relative.  */
  HOWTO (R_VISIUM_HI16_PCREL,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_HI16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* The low 16 bits of symbol value, pc relative.  */
  HOWTO (R_VISIUM_LO16_PCREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_LO16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* A 16 bit immediate value, pc relative.  */
  HOWTO (R_VISIUM_IM16_PCREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 visium_elf_howto_parity_reloc,	/* special_function */
	 "R_VISIUM_IM16_PCREL",	/* name */
	 false,			/* partial_inplace */
	 0x0000000,		/* src_mask */
	 0x000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

};

/* GNU extension to record C++ vtable hierarchy.  */
static reloc_howto_type visium_elf_vtinherit_howto =
  HOWTO (R_VISIUM_GNU_VTINHERIT,      /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 0,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_dont,   /* complain_on_overflow */
	 NULL,			   /* special_function */
	 "R_VISIUM_GNU_VTINHERIT", /* name */
	 false,			   /* partial_inplace */
	 0,			   /* src_mask */
	 0,			   /* dst_mask */
	 false);		   /* pcrel_offset */

/* GNU extension to record C++ vtable member usage.  */
static reloc_howto_type visium_elf_vtentry_howto =
  HOWTO (R_VISIUM_GNU_VTENTRY,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 0,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_dont,   /* complain_on_overflow */
	 NULL,			   /* special_function */
	 "R_VISIUM_GNU_VTENTRY",   /* name */
	 false,			   /* partial_inplace */
	 0,			   /* src_mask */
	 0,			   /* dst_mask */
	 false);		   /* pcrel_offset */

/* Return the parity bit for INSN shifted to its final position.  */

static bfd_vma
visium_parity_bit (bfd_vma insn)
{
  bfd_vma p = 0;
  int i;

  for (i = 0; i < 31; i++)
    {
      p ^= (insn & 1);
      insn >>= 1;
    }

  return p << 31;
}

/* This "special function" will only be used when the input and
   output files have different formats ie. when generating S-records
   directly using "--oformat srec". Otherwise we use
   _bfd_final_link_relocate which uses a howto structure, but does
   not use the special_function field.

   It sets instruction parity to even.  This cannot be done by a howto.  */

static bfd_reloc_status_type
visium_elf_howto_parity_reloc (bfd * input_bfd, arelent *reloc_entry,
			       asymbol *symbol, void *data,
			       asection *input_section, bfd *output_bfd,
			       char **error_message ATTRIBUTE_UNUSED)
{
  bfd_reloc_status_type ret;
  bfd_vma relocation;
  bfd_byte *inplace_address;
  bfd_vma insn;

  /* This part is from bfd_elf_generic_reloc.
     If we're relocating, and this an external symbol, we don't want
     to change anything.  */
  if (output_bfd != (bfd *) NULL && (symbol->flags & BSF_SECTION_SYM) == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* Now do the reloc in the usual way.  */

  /* Sanity check the address (offset in section).  */
  if (reloc_entry->address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  ret = bfd_reloc_ok;
  if (bfd_is_und_section (symbol->section) && output_bfd == (bfd *) NULL)
    ret = bfd_reloc_undefined;

  if (bfd_is_com_section (symbol->section) || output_bfd != (bfd *) NULL)
    relocation = 0;
  else
    relocation = symbol->value;

  /* Only do this for a final link.  */
  if (output_bfd == (bfd *) NULL)
    {
      relocation += symbol->section->output_section->vma;
      relocation += symbol->section->output_offset;
    }

  relocation += reloc_entry->addend;
  inplace_address = (bfd_byte *) data + reloc_entry->address;
  insn = bfd_get_32 (input_bfd, inplace_address);

  if (reloc_entry->howto->pc_relative)
    {
      relocation -= input_section->output_section->vma;
      relocation -= input_section->output_offset;
      relocation -= reloc_entry->address;
    }

  switch (reloc_entry->howto->type)
    {
    case R_VISIUM_PC16:
      if (ret == bfd_reloc_ok
	  && ((bfd_signed_vma) relocation < -0x20000
	      || (bfd_signed_vma) relocation > 0x1ffff))
	ret = bfd_reloc_overflow;
      relocation = (relocation >> 2) & 0xffff;
      break;
    case R_VISIUM_HI16:
    case R_VISIUM_HI16_PCREL:
      relocation = (relocation >> 16) & 0xffff;
      break;
    case R_VISIUM_LO16:
    case R_VISIUM_LO16_PCREL:
      relocation &= 0xffff;
      break;
    case R_VISIUM_IM16:
    case R_VISIUM_IM16_PCREL:
      if (ret == bfd_reloc_ok && (relocation & 0xffff0000) != 0)
	ret = bfd_reloc_overflow;
      relocation &= 0xffff;
      break;
    }
  insn = (insn & 0x7fff0000) | relocation;
  insn |= visium_parity_bit (insn);
  bfd_put_32 (input_bfd, insn, inplace_address);

  if (output_bfd != (bfd *) NULL)
    reloc_entry->address += input_section->output_offset;

  return ret;
}

static reloc_howto_type *
visium_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			  bfd_reloc_code_real_type code)
{
  /* Note that the visium_elf_howto_table is indexed by the R_
     constants. Thus, the order that the howto records appear in the
     table *must* match the order of the relocation types defined in
     include/elf/visium.h.  */
  switch (code)
    {
    case BFD_RELOC_NONE:
      return &visium_elf_howto_table[(int) R_VISIUM_NONE];
    case BFD_RELOC_8:
      return &visium_elf_howto_table[(int) R_VISIUM_8];
    case BFD_RELOC_16:
      return &visium_elf_howto_table[(int) R_VISIUM_16];
    case BFD_RELOC_32:
      return &visium_elf_howto_table[(int) R_VISIUM_32];
    case BFD_RELOC_8_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_8_PCREL];
    case BFD_RELOC_16_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_16_PCREL];
    case BFD_RELOC_32_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_32_PCREL];
    case BFD_RELOC_VISIUM_REL16:
      return &visium_elf_howto_table[(int) R_VISIUM_PC16];
    case BFD_RELOC_VISIUM_HI16:
      return &visium_elf_howto_table[(int) R_VISIUM_HI16];
    case BFD_RELOC_VISIUM_LO16:
      return &visium_elf_howto_table[(int) R_VISIUM_LO16];
    case BFD_RELOC_VISIUM_IM16:
      return &visium_elf_howto_table[(int) R_VISIUM_IM16];
    case BFD_RELOC_VISIUM_HI16_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_HI16_PCREL];
    case BFD_RELOC_VISIUM_LO16_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_LO16_PCREL];
    case BFD_RELOC_VISIUM_IM16_PCREL:
      return &visium_elf_howto_table[(int) R_VISIUM_IM16_PCREL];
    case BFD_RELOC_VTABLE_INHERIT:
      return &visium_elf_vtinherit_howto;
    case BFD_RELOC_VTABLE_ENTRY:
      return &visium_elf_vtentry_howto;
    default:
      return NULL;
    }
}

static reloc_howto_type *
visium_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (visium_elf_howto_table)
	    / sizeof (visium_elf_howto_table[0])); i++)
    if (visium_elf_howto_table[i].name != NULL
	&& strcasecmp (visium_elf_howto_table[i].name, r_name) == 0)
      return &visium_elf_howto_table[i];

  if (strcasecmp (visium_elf_vtinherit_howto.name, r_name) == 0)
    return &visium_elf_vtinherit_howto;
  if (strcasecmp (visium_elf_vtentry_howto.name, r_name) == 0)
    return &visium_elf_vtentry_howto;

  return NULL;
}

/* Set the howto pointer for a VISIUM ELF reloc.  */

static bool
visium_info_to_howto_rela (bfd *abfd, arelent *cache_ptr,
			   Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  switch (r_type)
    {
    case R_VISIUM_GNU_VTINHERIT:
      cache_ptr->howto = &visium_elf_vtinherit_howto;
      break;

    case R_VISIUM_GNU_VTENTRY:
      cache_ptr->howto = &visium_elf_vtentry_howto;
      break;

    default:
      if (r_type >= ARRAY_SIZE (visium_elf_howto_table))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      abfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      cache_ptr->howto = &visium_elf_howto_table[r_type];
      break;
    }
  return true;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
visium_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			 asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (bfd_link_relocatable (info))
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

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

      switch (ELF32_R_TYPE (rel->r_info))
	{
	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_VISIUM_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_VISIUM_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;
	}
    }

  return true;
}

/* Relocate a VISIUM ELF section.  */

static int
visium_elf_relocate_section (bfd *output_bfd,
			     struct bfd_link_info *info, bfd *input_bfd,
			     asection *input_section, bfd_byte *contents,
			     Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name = NULL;
      int r_type;
      bfd_vma insn;

      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type == R_VISIUM_GNU_VTINHERIT || r_type == R_VISIUM_GNU_VTENTRY)
	continue;

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto = visium_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* This is a local symbol.  */
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  bool unresolved_reloc;
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	{
	  /* For relocs against symbols from removed linkonce sections,
	     or sections discarded by a linker script, we just want the
	     section contents zeroed.  Avoid any special processing.  */
	  _bfd_clear_contents (howto, input_bfd, input_section,
			       contents, rel->r_offset);

	  rel->r_info = 0;
	  rel->r_addend = 0;
	  continue;
	}

      if (bfd_link_relocatable (info))
	continue;

      switch (r_type)
	{
	case R_VISIUM_PC16:
	case R_VISIUM_HI16:
	case R_VISIUM_LO16:
	case R_VISIUM_IM16:
	case R_VISIUM_HI16_PCREL:
	case R_VISIUM_LO16_PCREL:
	case R_VISIUM_IM16_PCREL:
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, rel->r_addend);

	  /* For instruction relocations, the parity needs correcting.  */
	  if (r == bfd_reloc_ok)
	    {
	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      insn = (insn & 0x7fffffff) | visium_parity_bit (insn);
	      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
	    }
	  break;

	default:
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, rel->r_addend);
	  break;
	}

      if (r != bfd_reloc_ok)
	{
	  const char *msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      (*info->callbacks->reloc_overflow)
		(info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      (*info->callbacks->undefined_symbol)
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg)
	    (*info->callbacks->warning) (info, msg, name, input_bfd,
					 input_section, rel->r_offset);
	}
    }

  return true;
}

/* This function is called during section gc to discover the section a
   to which a particular relocation refers.  Return the section that
   should be marked against GC for a given relocation.  */

static asection *
visium_elf_gc_mark_hook (asection *sec, struct bfd_link_info *info,
			 Elf_Internal_Rela *rel, struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_VISIUM_GNU_VTINHERIT:
      case R_VISIUM_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

static bool
visium_elf_init_file_header (bfd *abfd, struct bfd_link_info *info)
{
  Elf_Internal_Ehdr *i_ehdrp;

  if (!_bfd_elf_init_file_header (abfd, info))
    return false;

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_ABIVERSION] = 1;
  return true;
}

/* Function to set the ELF flag bits.  */

static bool
visium_elf_set_private_flags (bfd *abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Copy backend specific data from one object module to another.  */

static bool
visium_elf_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  BFD_ASSERT (!elf_flags_init (obfd)
	      || elf_elfheader (obfd)->e_flags ==
	      elf_elfheader (ibfd)->e_flags);

  elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
  elf_flags_init (obfd) = true;

  /* Copy object attributes.  */
  _bfd_elf_copy_obj_attributes (ibfd, obfd);

  return true;
}

/* Merge backend specific data from an object
   file to the output object file when linking.  */

static bool
visium_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags;
  flagword new_flags;
  flagword mismatch;
  const char *opt_arch = NULL;
  const char *new_opt_with = NULL;
  const char *old_opt_with = NULL;
  const char *with = "with";
  const char *without = "without";
  const char *mcm = "mcm";
  const char *mcm24 = "mcm24";
  const char *gr6 = "gr6";

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;
    }
  else
    {
      mismatch = (new_flags ^ old_flags)
	& (EF_VISIUM_ARCH_MCM | EF_VISIUM_ARCH_MCM24 | EF_VISIUM_ARCH_GR6);
      if (mismatch & EF_VISIUM_ARCH_GR6)
	{
	  opt_arch = gr6;
	  new_opt_with = new_flags & EF_VISIUM_ARCH_GR6 ? with : without;
	  old_opt_with = old_flags & EF_VISIUM_ARCH_GR6 ? with : without;
	}
      else if (mismatch & EF_VISIUM_ARCH_MCM)
	{
	  opt_arch = mcm;
	  new_opt_with = new_flags & EF_VISIUM_ARCH_MCM ? with : without;
	  old_opt_with = old_flags & EF_VISIUM_ARCH_MCM ? with : without;
	}
      else if (mismatch & EF_VISIUM_ARCH_MCM24)
	{
	  opt_arch = mcm24;
	  new_opt_with = new_flags & EF_VISIUM_ARCH_MCM24 ? with : without;
	  old_opt_with = old_flags & EF_VISIUM_ARCH_MCM24 ? with : without;
	}

      if (mismatch)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: compiled %s -mtune=%s and linked with modules"
	     " compiled %s -mtune=%s"),
	   ibfd, new_opt_with, opt_arch, old_opt_with, opt_arch);
    }

  return true;
}

static bool
visium_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;
  flagword flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (long) flags);

  if (flags & EF_VISIUM_ARCH_GR6)
    fprintf (file, " -mtune=gr6");
  else if (flags & EF_VISIUM_ARCH_MCM)
    fprintf (file, " -mtune=mcm");
  else if (flags & EF_VISIUM_ARCH_MCM24)
    fprintf (file, " -mtune=mcm24");

  fputc ('\n', file);
  return true;
}

#define ELF_ARCH		bfd_arch_visium
#define ELF_MACHINE_CODE	EM_VISIUM
#define ELF_OSABI		ELFOSABI_STANDALONE
#define ELF_MAXPAGESIZE		1

#define TARGET_BIG_SYM		visium_elf32_vec
#define TARGET_BIG_NAME		"elf32-visium"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			visium_info_to_howto_rela
#define elf_backend_relocate_section		visium_elf_relocate_section
#define elf_backend_gc_mark_hook		visium_elf_gc_mark_hook
#define elf_backend_check_relocs		visium_elf_check_relocs
#define elf_backend_rela_normal			1

#define elf_backend_can_gc_sections		1

#define bfd_elf32_bfd_reloc_type_lookup		visium_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		visium_reloc_name_lookup

#define bfd_elf32_bfd_set_private_flags		visium_elf_set_private_flags
#define bfd_elf32_bfd_copy_private_bfd_data	visium_elf_copy_private_bfd_data
#define bfd_elf32_bfd_merge_private_bfd_data	visium_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data	visium_elf_print_private_bfd_data
#define elf_backend_init_file_header		visium_elf_init_file_header

#include "elf32-target.h"
