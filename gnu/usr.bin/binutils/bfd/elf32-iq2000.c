/* IQ2000-specific support for 32-bit ELF.
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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/iq2000.h"
#include "libiberty.h"

/* Forward declarations.  */

static bfd_reloc_status_type iq2000_elf_howto_hi16_reloc (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);


static reloc_howto_type iq2000_elf_howto_table [] =
{
  /* This reloc does nothing.  */

  HOWTO (R_IQ2000_NONE,		     /* type */
	 0,			     /* rightshift */
	 0,			     /* size */
	 0,			     /* bitsize */
	 false,			     /* pc_relative */
	 0,			     /* bitpos */
	 complain_overflow_dont,     /* complain_on_overflow */
	 bfd_elf_generic_reloc,	     /* special_function */
	 "R_IQ2000_NONE",	     /* name */
	 false,			     /* partial_inplace */
	 0,			     /* src_mask */
	 0,			     /* dst_mask */
	 false),		     /* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_IQ2000_16,		     /* type */
	 0,			     /* rightshift */
	 2,			     /* size */
	 16,			     /* bitsize */
	 false,			     /* pc_relative */
	 0,			     /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	     /* special_function */
	 "R_IQ2000_16",		     /* name */
	 false,			     /* partial_inplace */
	 0x0000,		     /* src_mask */
	 0xffff,		     /* dst_mask */
	 false),		     /* pcrel_offset */

  /* A 32 bit absolute relocation.  */
  HOWTO (R_IQ2000_32,		     /* type */
	 0,			     /* rightshift */
	 4,			     /* size */
	 31,			     /* bitsize */
	 false,			     /* pc_relative */
	 0,			     /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	     /* special_function */
	 "R_IQ2000_32",		     /* name */
	 false,			     /* partial_inplace */
	 0x00000000,		     /* src_mask */
	 0x7fffffff,		     /* dst_mask */
	 false),		     /* pcrel_offset */

  /* 26 bit branch address.  */
  HOWTO (R_IQ2000_26,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
				/* This needs complex overflow
				   detection, because the upper four
				   bits must match the PC.  */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_26",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x03ffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16 bit PC relative reference.  */
  HOWTO (R_IQ2000_PC16,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_PC16",	/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  /* high 16 bits of symbol value.  */
  HOWTO (R_IQ2000_HI16,		/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 iq2000_elf_howto_hi16_reloc,	/* special_function */
	 "R_IQ2000_HI16",	/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0x7fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* Low 16 bits of symbol value.  */
  HOWTO (R_IQ2000_LO16,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_LO16",	/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 16-bit jump offset.  */
  HOWTO (R_IQ2000_OFFSET_16,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_OFFSET_16",	/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 21-bit jump offset.  */
  HOWTO (R_IQ2000_OFFSET_21,	/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 21,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_OFFSET_21",	/* name */
	 false,			/* partial_inplace */
	 0x000000,		/* src_mask */
	 0x1fffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* unsigned high 16 bits of value.  */
  HOWTO (R_IQ2000_OFFSET_21,	/* type */
	 16,			/* rightshift */
	 4,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_IQ2000_UHI16",	/* name */
	 false,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0x7fff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 32 bit absolute debug relocation.  */
  HOWTO (R_IQ2000_32_DEBUG,	     /* type */
	 0,			     /* rightshift */
	 4,			     /* size */
	 32,			     /* bitsize */
	 false,			     /* pc_relative */
	 0,			     /* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	     /* special_function */
	 "R_IQ2000_32",		     /* name */
	 false,			     /* partial_inplace */
	 0x00000000,		     /* src_mask */
	 0xffffffff,		     /* dst_mask */
	 false),		     /* pcrel_offset */

};

/* GNU extension to record C++ vtable hierarchy.  */
static reloc_howto_type iq2000_elf_vtinherit_howto =
  HOWTO (R_IQ2000_GNU_VTINHERIT,    /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 0,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_dont,   /* complain_on_overflow */
	 NULL,			   /* special_function */
	 "R_IQ2000_GNU_VTINHERIT",  /* name */
	 false,			   /* partial_inplace */
	 0,			   /* src_mask */
	 0,			   /* dst_mask */
	 false);		   /* pcrel_offset */

/* GNU extension to record C++ vtable member usage.  */
static reloc_howto_type iq2000_elf_vtentry_howto =
  HOWTO (R_IQ2000_GNU_VTENTRY,	   /* type */
	 0,			   /* rightshift */
	 4,			   /* size */
	 0,			   /* bitsize */
	 false,			   /* pc_relative */
	 0,			   /* bitpos */
	 complain_overflow_dont,   /* complain_on_overflow */
	 NULL,			   /* special_function */
	 "R_IQ2000_GNU_VTENTRY",    /* name */
	 false,			   /* partial_inplace */
	 0,			   /* src_mask */
	 0,			   /* dst_mask */
	 false);		   /* pcrel_offset */


static bfd_reloc_status_type
iq2000_elf_howto_hi16_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			     arelent *reloc_entry,
			     asymbol *symbol,
			     void * data,
			     asection *input_section,
			     bfd *output_bfd,
			     char **error_message ATTRIBUTE_UNUSED)
{
  bfd_reloc_status_type ret;
  bfd_vma relocation;

  /* If we're relocating and this an external symbol,
     we don't want to change anything.  */
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  relocation += symbol->section->output_section->vma;
  relocation += symbol->section->output_offset;
  relocation += reloc_entry->addend;

  /* If %lo will have sign-extension, compensate by add 0x10000 to hi portion.  */
  if (relocation & 0x8000)
    reloc_entry->addend += 0x10000;

  /* Now do the reloc in the usual way.	 */
  ret = bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				input_section, output_bfd, error_message);

  /* Put it back the way it was.  */
  if (relocation & 0x8000)
    reloc_entry->addend -= 0x10000;

  return ret;
}

static bfd_reloc_status_type
iq2000_elf_relocate_hi16 (bfd *input_bfd,
			  Elf_Internal_Rela *relhi,
			  bfd_byte *contents,
			  bfd_vma value)
{
  bfd_vma insn;

  insn = bfd_get_32 (input_bfd, contents + relhi->r_offset);

  value += relhi->r_addend;
  value &= 0x7fffffff; /* Mask off top-bit which is Harvard mask bit.  */

  /* If top-bit of %lo value is on, this means that %lo will
     sign-propagate and so we compensate by adding 1 to %hi value.  */
  if (value & 0x8000)
    value += 0x10000;

  value >>= 16;
  insn = ((insn & ~0xFFFF) | value);

  bfd_put_32 (input_bfd, insn, contents + relhi->r_offset);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
iq2000_elf_relocate_offset16 (bfd *input_bfd,
			      Elf_Internal_Rela *rel,
			      bfd_byte *contents,
			      bfd_vma value,
			      bfd_vma location)
{
  bfd_vma insn;
  bfd_vma jtarget;

  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);

  value += rel->r_addend;

  if (value & 3)
    return bfd_reloc_dangerous;

  jtarget = (value & 0x3fffc) | (location & 0xf0000000L);

  if (jtarget != value)
    return bfd_reloc_overflow;

  insn = (insn & ~0xFFFF) | ((value >> 2) & 0xFFFF);

  bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
  return bfd_reloc_ok;
}

/* Map BFD reloc types to IQ2000 ELF reloc types.  */

static reloc_howto_type *
iq2000_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			  bfd_reloc_code_real_type code)
{
  /* Note that the iq2000_elf_howto_table is indxed by the R_
     constants.	 Thus, the order that the howto records appear in the
     table *must* match the order of the relocation types defined in
     include/elf/iq2000.h.  */

  switch (code)
    {
    case BFD_RELOC_NONE:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_NONE];
    case BFD_RELOC_16:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_16];
    case BFD_RELOC_32:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_32];
    case BFD_RELOC_MIPS_JMP:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_26];
    case BFD_RELOC_IQ2000_OFFSET_16:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_OFFSET_16];
    case BFD_RELOC_IQ2000_OFFSET_21:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_OFFSET_21];
    case BFD_RELOC_16_PCREL_S2:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_PC16];
    case BFD_RELOC_HI16:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_HI16];
    case BFD_RELOC_IQ2000_UHI16:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_UHI16];
    case BFD_RELOC_LO16:
      return &iq2000_elf_howto_table[ (int) R_IQ2000_LO16];
    case BFD_RELOC_VTABLE_INHERIT:
      return &iq2000_elf_vtinherit_howto;
    case BFD_RELOC_VTABLE_ENTRY:
      return &iq2000_elf_vtentry_howto;
    default:
      /* Pacify gcc -Wall.  */
      return NULL;
    }
  return NULL;
}

static reloc_howto_type *
iq2000_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (iq2000_elf_howto_table)
	    / sizeof (iq2000_elf_howto_table[0]));
       i++)
    if (iq2000_elf_howto_table[i].name != NULL
	&& strcasecmp (iq2000_elf_howto_table[i].name, r_name) == 0)
      return &iq2000_elf_howto_table[i];

  if (strcasecmp (iq2000_elf_vtinherit_howto.name, r_name) == 0)
    return &iq2000_elf_vtinherit_howto;
  if (strcasecmp (iq2000_elf_vtentry_howto.name, r_name) == 0)
    return &iq2000_elf_vtentry_howto;

  return NULL;
}

/* Perform a single relocation.	 By default we use the standard BFD
   routines.  */

static bfd_reloc_status_type
iq2000_final_link_relocate (reloc_howto_type *	howto,
			    bfd *		input_bfd,
			    asection *		input_section,
			    bfd_byte *		contents,
			    Elf_Internal_Rela *	rel,
			    bfd_vma		relocation)
{
  return _bfd_final_link_relocate (howto, input_bfd, input_section,
				   contents, rel->r_offset,
				   relocation, rel->r_addend);
}

/* Set the howto pointer for a IQ2000 ELF reloc.  */

static bool
iq2000_info_to_howto_rela (bfd * abfd ATTRIBUTE_UNUSED,
			   arelent * cache_ptr,
			   Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  switch (r_type)
    {
    case R_IQ2000_GNU_VTINHERIT:
      cache_ptr->howto = & iq2000_elf_vtinherit_howto;
      break;

    case R_IQ2000_GNU_VTENTRY:
      cache_ptr->howto = & iq2000_elf_vtentry_howto;
      break;

    default:
      if (r_type >= ARRAY_SIZE (iq2000_elf_howto_table))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      abfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      cache_ptr->howto = & iq2000_elf_howto_table [r_type];
      break;
    }
  return true;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.	 */

static bool
iq2000_elf_check_relocs (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bool changed = false;

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
	  /* This relocation describes the C++ object vtable
	     hierarchy.  Reconstruct it for later use during GC.  */
	case R_IQ2000_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries
	     are actually used.  Record for later use during GC.  */
	case R_IQ2000_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	case R_IQ2000_32:
	  /* For debug section, change to special harvard-aware relocations.  */
	  if (startswith (sec->name, ".debug")
	      || startswith (sec->name, ".stab")
	      || startswith (sec->name, ".eh_frame"))
	    {
	      ((Elf_Internal_Rela *) rel)->r_info
		= ELF32_R_INFO (ELF32_R_SYM (rel->r_info), R_IQ2000_32_DEBUG);
	      changed = true;
	    }
	  break;
	}
    }

  if (changed)
    /* Note that we've changed relocs, otherwise if !info->keep_memory
       we'll free the relocs and lose our changes.  */
    elf_section_data (sec)->relocs = (Elf_Internal_Rela *) relocs;

  return true;
}


/* Relocate a IQ2000 ELF section.
   There is some attempt to make this function usable for many architectures,
   both USE_REL and USE_RELA ['twould be nice if such a critter existed],
   if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjusting the section contents as
   necessary, and (if using Rela relocs and generating a relocatable
   output file) adjusting the reloc addend as necessary.

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
   accordingly.	 */

static int
iq2000_elf_relocate_section (bfd *		     output_bfd ATTRIBUTE_UNUSED,
			     struct bfd_link_info *  info,
			     bfd *		     input_bfd,
			     asection *		     input_section,
			     bfd_byte *		     contents,
			     Elf_Internal_Rela *     relocs,
			     Elf_Internal_Sym *	     local_syms,
			     asection **	     local_sections)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *		rel;
  Elf_Internal_Rela *		relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *	   howto;
      unsigned long		   r_symndx;
      Elf_Internal_Sym *	   sym;
      asection *		   sec;
      struct elf_link_hash_entry * h;
      bfd_vma			   relocation;
      bfd_reloc_status_type	   r;
      const char *		   name = NULL;
      int			   r_type;

      r_type = ELF32_R_TYPE (rel->r_info);

      if (   r_type == R_IQ2000_GNU_VTINHERIT
	  || r_type == R_IQ2000_GNU_VTENTRY)
	continue;

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = iq2000_elf_howto_table + ELF32_R_TYPE (rel->r_info);
      h	     = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  asection *osec;

	  sym = local_syms + r_symndx;
	  osec = sec = local_sections [r_symndx];
	  if ((sec->flags & SEC_MERGE)
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    /* This relocation is relative to a section symbol that is
	       going to be merged.  Change it so that it is relative
	       to the merged section symbol.  */
	    rel->r_addend = _bfd_elf_rel_local_sym (output_bfd, sym, &sec,
						    rel->r_addend);

	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (osec) : name;
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
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      switch (r_type)
	{
	case R_IQ2000_HI16:
	  r = iq2000_elf_relocate_hi16 (input_bfd, rel, contents, relocation);
	  break;

	case R_IQ2000_OFFSET_16:
	  r = iq2000_elf_relocate_offset16 (input_bfd, rel, contents, relocation,
					    input_section->output_section->vma
					    + input_section->output_offset
					    + rel->r_offset);
	  break;

	case R_IQ2000_PC16:
	  rel->r_addend -= 4;
	  /* Fall through.  */

	default:
	  r = iq2000_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel, relocation);
	  break;
	}

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) NULL;

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


/* Return the section that should be marked against GC for a given
   relocation.	*/

static asection *
iq2000_elf_gc_mark_hook (asection *sec,
			 struct bfd_link_info *info,
			 Elf_Internal_Rela *rel,
			 struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_IQ2000_GNU_VTINHERIT:
      case R_IQ2000_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}


/* Return the MACH for an e_flags value.  */

static int
elf32_iq2000_machine (bfd *abfd)
{
  switch (elf_elfheader (abfd)->e_flags & EF_IQ2000_CPU_MASK)
    {
    case EF_IQ2000_CPU_IQ10:
      return bfd_mach_iq10;

    case EF_IQ2000_CPU_IQ2000:
    default:
      return bfd_mach_iq2000;
    }
}


/* Function to set the ELF flag bits.  */

static bool
iq2000_elf_set_private_flags (bfd *abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Merge backend specific data from an object
   file to the output object file when linking.  */

static bool
iq2000_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword old_flags, old_partial;
  flagword new_flags, new_partial;
  bool error = false;
  char new_opt[80];
  char old_opt[80];

  new_opt[0] = old_opt[0] = '\0';
  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* First call, no flags set.  */
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;
    }

  else if (new_flags != old_flags)
    {
      /* Warn if different cpu is used, but allow a
	 specific cpu to override the generic cpu.  */
      new_partial = (new_flags & EF_IQ2000_CPU_MASK);
      old_partial = (old_flags & EF_IQ2000_CPU_MASK);

      if (new_partial != old_partial)
	{
	  switch (new_partial)
	    {
	    case EF_IQ2000_CPU_IQ10:
	      strcat (new_opt, " -m10");
	      break;

	    default:
	    case EF_IQ2000_CPU_IQ2000:
	      strcat (new_opt, " -m2000");
	      break;
	    }

	  switch (old_partial)
	    {
	    case EF_IQ2000_CPU_IQ10:
	      strcat (old_opt, " -m10");
	      break;

	    default:
	    case EF_IQ2000_CPU_IQ2000:
	      strcat (old_opt, " -m2000");
	      break;
	    }
	}

      /* Print out any mismatches from above.  */
      if (new_opt[0])
	{
	  error = true;
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: compiled with %s and linked with modules compiled with %s"),
	     ibfd, new_opt, old_opt);
	}

      new_flags &= ~ EF_IQ2000_ALL_FLAGS;
      old_flags &= ~ EF_IQ2000_ALL_FLAGS;

      /* Warn about any other mismatches.  */
      if (new_flags != old_flags)
	{
	  error = true;

	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: uses different e_flags (%#x) fields than previous modules (%#x)"),
	     ibfd, new_flags, old_flags);
	}
    }

  if (error)
    bfd_set_error (bfd_error_bad_value);

  return !error;
}


static bool
iq2000_elf_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;
  flagword flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (unsigned long) flags);

  switch (flags & EF_IQ2000_CPU_MASK)
    {
    case EF_IQ2000_CPU_IQ10:
      fprintf (file, " -m10");
      break;
    case EF_IQ2000_CPU_IQ2000:
      fprintf (file, " -m2000");
      break;
    default:
      break;
    }

  fputc ('\n', file);
  return true;
}

static
bool
iq2000_elf_object_p (bfd *abfd)
{
  bfd_default_set_arch_mach (abfd, bfd_arch_iq2000,
			     elf32_iq2000_machine (abfd));
  return true;
}


#define ELF_ARCH		bfd_arch_iq2000
#define ELF_MACHINE_CODE	EM_IQ2000
#define ELF_MAXPAGESIZE		0x1000

#define TARGET_BIG_SYM		iq2000_elf32_vec
#define TARGET_BIG_NAME		"elf32-iq2000"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			iq2000_info_to_howto_rela
#define elf_backend_relocate_section		iq2000_elf_relocate_section
#define elf_backend_gc_mark_hook		iq2000_elf_gc_mark_hook
#define elf_backend_check_relocs		iq2000_elf_check_relocs
#define elf_backend_object_p			iq2000_elf_object_p
#define elf_backend_rela_normal			1

#define elf_backend_can_gc_sections		1

#define bfd_elf32_bfd_reloc_type_lookup		iq2000_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	iq2000_reloc_name_lookup
#define bfd_elf32_bfd_set_private_flags		iq2000_elf_set_private_flags
#define bfd_elf32_bfd_merge_private_bfd_data	iq2000_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data	iq2000_elf_print_private_bfd_data

#include "elf32-target.h"
