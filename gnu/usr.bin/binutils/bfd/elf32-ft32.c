/* ft32-specific support for 32-bit ELF.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.

   Copied from elf32-moxie.c which is..
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Free Software Foundation, Inc.

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
#include "elf/ft32.h"
#include "opcode/ft32.h"

static bool debug_relax = false;

static bfd_reloc_status_type
bfd_elf_ft32_diff_reloc (bfd *, arelent *, asymbol *, void *,
			asection *, bfd *, char **);

static reloc_howto_type ft32_elf_howto_table [] =
{
  /* This reloc does nothing.  */
  HOWTO (R_FT32_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* A 32 bit absolute relocation.  */

  HOWTO (R_FT32_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_32",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_16",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_8",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_10,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 false,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_10",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00003ff0,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_20,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 20,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_20",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x000fffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_17,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 17,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_17",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0001ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_18,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 18,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_18",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x0003ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_RELAX,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 false,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_RELAX",	/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_FT32_SC0,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 10,			/* bitsize */
	 false,			/* pc_relative */
	 4,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_SC0",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_FT32_SC1,		/* type */
	 2,			/* rightshift */
	 4,			/* size */
	 22,			/* bitsize */
	 true,			/* pc_relative */
	 7,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_SC1",		/* name */
	 true,			/* partial_inplace */
	 0x07ffff80,		/* src_mask */
	 0x07ffff80,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_FT32_15,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 15,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_FT32_15",		/* name */
	 false,			/* partial_inplace */
	 0x00000000,		/* src_mask */
	 0x00007fff,		/* dst_mask */
	 false),		/* pcrel_offset */
  HOWTO (R_FT32_DIFF32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_ft32_diff_reloc, /* special_function */
	 "R_FT32_DIFF32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
};

/* Map BFD reloc types to FT32 ELF reloc types.  */

struct ft32_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int ft32_reloc_val;
};

static const struct ft32_reloc_map ft32_reloc_map [] =
{
  { BFD_RELOC_NONE,		R_FT32_NONE },
  { BFD_RELOC_32,		R_FT32_32 },
  { BFD_RELOC_16,		R_FT32_16 },
  { BFD_RELOC_8,		R_FT32_8 },
  { BFD_RELOC_FT32_10,		R_FT32_10 },
  { BFD_RELOC_FT32_20,		R_FT32_20 },
  { BFD_RELOC_FT32_17,		R_FT32_17 },
  { BFD_RELOC_FT32_18,		R_FT32_18 },
  { BFD_RELOC_FT32_RELAX,	R_FT32_RELAX },
  { BFD_RELOC_FT32_SC0,		R_FT32_SC0 },
  { BFD_RELOC_FT32_SC1,		R_FT32_SC1 },
  { BFD_RELOC_FT32_15,		R_FT32_15 },
  { BFD_RELOC_FT32_DIFF32,	R_FT32_DIFF32 },
};

/* Perform a diff relocation. Nothing to do, as the difference value is
   already written into the section's contents. */

static bfd_reloc_status_type
bfd_elf_ft32_diff_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		      arelent *reloc_entry ATTRIBUTE_UNUSED,
	      asymbol *symbol ATTRIBUTE_UNUSED,
	      void *data ATTRIBUTE_UNUSED,
	      asection *input_section ATTRIBUTE_UNUSED,
	      bfd *output_bfd ATTRIBUTE_UNUSED,
	      char **error_message ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}

static reloc_howto_type *
ft32_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (ft32_reloc_map) / sizeof (ft32_reloc_map[0]); i++)
    if (ft32_reloc_map [i].bfd_reloc_val == code)
      return & ft32_elf_howto_table [ft32_reloc_map[i].ft32_reloc_val];

  return NULL;
}

static reloc_howto_type *
ft32_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (ft32_elf_howto_table) / sizeof (ft32_elf_howto_table[0]);
       i++)
    if (ft32_elf_howto_table[i].name != NULL
	&& strcasecmp (ft32_elf_howto_table[i].name, r_name) == 0)
      return &ft32_elf_howto_table[i];

  return NULL;
}

/* Set the howto pointer for an FT32 ELF reloc.  */

static bool
ft32_info_to_howto_rela (bfd *abfd,
			  arelent *cache_ptr,
			  Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_FT32_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = & ft32_elf_howto_table [r_type];
  return cache_ptr->howto != NULL;
}

/* Relocate an FT32 ELF section.

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
   accordingly.  */

static int
ft32_elf_relocate_section (bfd *output_bfd,
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
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name;
      int r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto  = ft32_elf_howto_table + r_type;
      h      = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = name == NULL ? bfd_section_name (sec) : name;
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;

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

      switch (howto->type)
	{
	  case R_FT32_SC0:
	    {
	      unsigned int insn;
	      int offset;
	      unsigned int code15[2];

	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      ft32_split_shortcode (insn, code15);

	      offset = (int)relocation;
	      offset += (int)(rel->r_addend - rel->r_offset);
	      offset -= (input_section->output_section->vma +
			 input_section->output_offset);
	      if ((offset < -1024) || (offset >= 1024))
		{
		  r = bfd_reloc_outofrange;
		  break;
		}
	      code15[0] |= ((offset / 4) & 511);
	      insn = ft32_merge_shortcode (code15);
	      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
	    }
	    r = bfd_reloc_ok;
	    break;

	  case R_FT32_SC1:
	    {
	      unsigned int insn;
	      int offset;
	      unsigned int code15[2];

	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      ft32_split_shortcode (insn, code15);

	      offset = (int)relocation;
	      offset += (int)(rel->r_addend - rel->r_offset);
	      offset -= (input_section->output_section->vma +
			 input_section->output_offset);
	      if ((offset < -1024) || (offset >= 1024))
		{
		  r = bfd_reloc_outofrange;
		  break;
		}
	      code15[1] |= ((offset / 4) & 511);
	      insn = ft32_merge_shortcode (code15);
	      bfd_put_32 (input_bfd, insn, contents + rel->r_offset);
	    }
	    r = bfd_reloc_ok;
	    break;

	  case R_FT32_DIFF32:
	    r = bfd_reloc_ok;
	    break;

	  default:
	    r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					  contents, rel->r_offset,
					  relocation, rel->r_addend);
	    break;
	}

      if (r != bfd_reloc_ok)
	{
	  const char * msg = NULL;

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

/* Relaxation.  */

static bool
ft32_reloc_shortable
    (bfd *		    abfd,
     asection *		    sec,
     Elf_Internal_Sym *	    isymbuf ATTRIBUTE_UNUSED,
     bfd_byte *		    contents,
     bfd_vma		    pc ATTRIBUTE_UNUSED,
     Elf_Internal_Rela *    irel,
     unsigned int *	    sc)
{
  Elf_Internal_Shdr *symtab_hdr ATTRIBUTE_UNUSED;
  bfd_vma symval;

  enum elf_ft32_reloc_type r_type;
  reloc_howto_type *howto = NULL;
  unsigned int insn;
  int offset;
  bfd_vma dot, value;

  r_type = ELF32_R_TYPE (irel->r_info);
  howto = &ft32_elf_howto_table [r_type];

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Get the value of the symbol referred to by the reloc.  */
  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
    {
      /* A local symbol.  */
      Elf_Internal_Sym *isym;
      asection *sym_sec;

      isym = isymbuf + ELF32_R_SYM (irel->r_info);
      sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      symval = isym->st_value;
      /* If the reloc is absolute, it will not have
	 a symbol or section associated with it.  */
      if (sym_sec)
	symval += sym_sec->output_section->vma
	  + sym_sec->output_offset;
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
	return false;

      symval = (h->root.u.def.value
		+ h->root.u.def.section->output_section->vma
		+ h->root.u.def.section->output_offset);
    }

  switch (r_type)
    {
      case R_FT32_8:
      case R_FT32_10:
      case R_FT32_16:
      case R_FT32_20:
      case R_FT32_RELAX:
	if (symval != 0)
	  return false;
	insn = bfd_get_32 (abfd, contents + irel->r_offset);
	insn |= ((symval + irel->r_addend) << howto->bitpos) & howto->dst_mask;
	return ft32_shortcode (insn, sc);

      case R_FT32_18:
	insn = bfd_get_32 (abfd, contents + irel->r_offset);
	/* Get the address of this instruction.  */
	dot = (sec->output_section->vma
	       + sec->output_offset + irel->r_offset);
	value = symval + irel->r_addend;
	offset = (value - dot) / 4;

	if ((dot > 0x8c) && (-256 <= offset) && (offset < 256))
	  {
	    switch (insn)
	      {
		case 0x00200000: *sc = (3 << 13) | (0  << 9); return true;
		case 0x00280000: *sc = (3 << 13) | (1  << 9); return true;
		case 0x00600000: *sc = (3 << 13) | (2  << 9); return true;
		case 0x00680000: *sc = (3 << 13) | (3  << 9); return true;
		case 0x00a00000: *sc = (3 << 13) | (4  << 9); return true;
		case 0x00a80000: *sc = (3 << 13) | (5  << 9); return true;
		case 0x00e00000: *sc = (3 << 13) | (6  << 9); return true;
		case 0x00e80000: *sc = (3 << 13) | (7  << 9); return true;
		case 0x01200000: *sc = (3 << 13) | (8  << 9); return true;
		case 0x01280000: *sc = (3 << 13) | (9  << 9); return true;
		case 0x01600000: *sc = (3 << 13) | (10 << 9); return true;
		case 0x01680000: *sc = (3 << 13) | (11 << 9); return true;
		case 0x01a00000: *sc = (3 << 13) | (12 << 9); return true;
		case 0x01a80000: *sc = (3 << 13) | (13 << 9); return true;

		case 0x00300000: *sc = (3 << 13) | (14 << 9); return true;
		case 0x00340000: *sc = (3 << 13) | (15 << 9); return true;

		default:
		  break;
	      }
	  }
	break;

      default:
	break;
    }
  return false;
}

/* Returns whether the relocation type passed is a diff reloc.  */

static bool
elf32_ft32_is_diff_reloc (Elf_Internal_Rela *irel)
{
  return (ELF32_R_TYPE (irel->r_info) == R_FT32_DIFF32);
}

/* Reduce the diff value written in the section by count if the shrinked
   insn address happens to fall between the two symbols for which this
   diff reloc was emitted.  */

static bool
elf32_ft32_adjust_diff_reloc_value (bfd *abfd,
				   struct bfd_section *isec,
				   Elf_Internal_Rela *irel,
				   bfd_vma symval,
				   bfd_vma shrinked_insn_address,
				   int count)
{
  unsigned char * reloc_contents = NULL;
  unsigned char * isec_contents = elf_section_data (isec)->this_hdr.contents;
  bfd_signed_vma x = 0;
  bfd_vma sym2_address;
  bfd_vma sym1_address;
  bfd_vma start_address;
  bfd_vma end_address;


  if (isec_contents == NULL)
    {
      if (! bfd_malloc_and_get_section (abfd, isec, &isec_contents))
	return false;

      elf_section_data (isec)->this_hdr.contents = isec_contents;
    }

  reloc_contents = isec_contents + irel->r_offset;

  /* Read value written in object file.  */
  switch (ELF32_R_TYPE (irel->r_info))
    {
    case R_FT32_DIFF32:
      x = bfd_get_signed_32 (abfd, reloc_contents);
      break;

    default:
      return false;
    }

  /* For a diff reloc sym1 - sym2 the diff at assembly time (x) is written
     into the object file at the reloc offset. sym2's logical value is
     symval (<start_of_section>) + reloc addend. Compute the start and end
     addresses and check if the shrinked insn falls between sym1 and sym2.  */
  sym2_address = symval + irel->r_addend;
  sym1_address = sym2_address - x;

  /* Don't assume sym2 is bigger than sym1 - the difference
     could be negative. Compute start and end addresses, and
     use those to see if they span shrinked_insn_address.  */
  start_address = sym1_address < sym2_address ? sym1_address : sym2_address;
  end_address = sym1_address > sym2_address ? sym1_address : sym2_address;

  if (shrinked_insn_address >= start_address
      && shrinked_insn_address < end_address)
    {
      /* Reduce the diff value by count bytes and write it back into section
	 contents.  */
      bfd_signed_vma new_diff = x < 0 ? x + count : x - count;

      if (sym2_address > shrinked_insn_address)
	irel->r_addend -= count;

      switch (ELF32_R_TYPE (irel->r_info))
	{
	case R_FT32_DIFF32:
	  bfd_put_signed_32 (abfd, new_diff & 0xFFFFFFFF, reloc_contents);
	  break;

	default:
	  return false;
	}
    }

  return true;
}

static bool
elf32_ft32_adjust_reloc_if_spans_insn (bfd *abfd,
				      asection *isec,
				      Elf_Internal_Rela *irel,  bfd_vma symval,
				      bfd_vma shrinked_insn_address,
				      bfd_vma shrink_boundary,
				      int count)
{

  if (elf32_ft32_is_diff_reloc (irel))
    {
      if (!elf32_ft32_adjust_diff_reloc_value (abfd, isec, irel,
					       symval,
					       shrinked_insn_address,
					       count))
	return false;
    }
  else
    {
      bfd_vma reloc_value = symval + irel->r_addend;
      bool addend_within_shrink_boundary =
	(reloc_value <= shrink_boundary);
      bool reloc_spans_insn =
	(symval <= shrinked_insn_address
	 && reloc_value > shrinked_insn_address
	 && addend_within_shrink_boundary);

      if (! reloc_spans_insn)
	return true;

      irel->r_addend -= count;

      if (debug_relax)
	printf ("Relocation's addend needed to be fixed \n");
    }
  return true;
}

/* Delete some bytes from a section while relaxing.  */

static bool
elf32_ft32_relax_delete_bytes (struct bfd_link_info *link_info, bfd * abfd,
			       asection * sec, bfd_vma addr, int count)
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
  Elf_Internal_Sym *isymbuf = NULL;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
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

  /* The reloc's own addresses are now ok. However, we need to readjust
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
				 + sec->output_offset + addr - count);

	irel = elf_section_data (isec)->relocs;
	/* PR 12161: Read in the relocs for this section if necessary.  */
	if (irel == NULL)
	  irel = _bfd_elf_link_read_relocs (abfd, isec, NULL, NULL, true);

	for (irelend = irel + isec->reloc_count; irel < irelend; irel++)
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

		    if (symval <= shrinked_insn_address
			&& (symval + irel->r_addend) > shrinked_insn_address)
		      {
			/* If there is an alignment boundary, we only need to
			   adjust addends that end up below the boundary. */
			bfd_vma shrink_boundary = (toaddr
						   + sec->output_section->vma
						   + sec->output_offset);

			if (debug_relax)
			  printf
			    ("Relocation's addend needed to be fixed \n");

			if (!elf32_ft32_adjust_reloc_if_spans_insn (abfd, isec,
								    irel, symval,
								    shrinked_insn_address,
								    shrink_boundary,
								    count))
			  return false;
		      }
		  }
		/* else reference symbol is absolute. No adjustment needed. */
	      }
	    /* else...Reference symbol is extern.  No need for adjusting
	       the addend.  */
	  }
      }
  }

  /* Adjust the local symbols defined in this section.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  if (isym)
    {
      for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
	{
	  if (isym->st_shndx == sec_shndx
	      && isym->st_value > addr && isym->st_value < toaddr)
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

/* Return TRUE if LOC can be a target of a branch, jump or call.  */

static bool
elf32_ft32_relax_is_branch_target (struct bfd_link_info *link_info,
				   bfd * abfd, asection * sec,
				   bfd_vma loc)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *irel, *irelend;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymbuf = NULL;
  bfd_vma symval;
  struct bfd_section *isec;

  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  struct elf_link_hash_entry **start_hashes;
  unsigned int symcount;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Now we check for relocations pointing to ret.  */
  for (isec = abfd->sections; isec; isec = isec->next)
    {
      irel = elf_section_data (isec)->relocs;
      if (irel == NULL)
	irel = _bfd_elf_link_read_relocs (abfd, isec, NULL, NULL, true);

      irelend = irel + isec->reloc_count;

      for (; irel < irelend; irel++)
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
		    printf ("0x%x: Address of anchor symbol: 0x%x "
			    "Address of relocation target: 0x%x \n",
			    (unsigned int) irel->r_offset,
			    (unsigned int) symval,
			    (unsigned int) (symval + irel->r_addend));
		  if ((irel->r_addend) == loc)
		    return true;
		}
	    }
	}
    }

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
	  && sym_hash->root.u.def.value == loc)
	return true;
    }

  return false;
}

static bool
ft32_elf_relax_section (bfd *abfd,
			asection *sec,
			struct bfd_link_info *link_info,
			bool *again)
{
  Elf_Internal_Rela * free_relocs = NULL;
  Elf_Internal_Rela * internal_relocs;
  Elf_Internal_Rela * irelend;
  Elf_Internal_Rela * irel;
  bfd_byte *	      contents = NULL;
  Elf_Internal_Shdr * symtab_hdr;
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

  /* Get the section contents.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    contents = elf_section_data (sec)->this_hdr.contents;
  /* Go get them off disk.  */
  else
    {
      if (! bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;
      elf_section_data (sec)->this_hdr.contents = contents;
    }

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

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
      symtab_hdr->contents = (unsigned char *) isymbuf;
    }

  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL,
					       link_info->keep_memory);
  if (internal_relocs == NULL)
    goto error_return;
  if (! link_info->keep_memory)
    free_relocs = internal_relocs;

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;

  /* Test every adjacent pair of relocs. If both have shortcodes,
     fuse them and delete the relocs.  */
  irel = internal_relocs;
  while (irel < irelend - 1)
    {
      Elf_Internal_Rela * irel_next = irel + 1;
      unsigned int sc0, sc1;
      bfd_vma pc;

      pc = irel->r_offset;

      if (((pc + 4) == (irel_next->r_offset))
	  && ft32_reloc_shortable (abfd, sec, isymbuf, contents, pc, irel,
				   &sc0)
	  && ft32_reloc_shortable (abfd, sec, isymbuf, contents, pc,
				   irel_next, &sc1)
	  && !elf32_ft32_relax_is_branch_target (link_info, abfd, sec,
						 irel_next->r_offset))
	{
	  unsigned int code30 = (sc1 << 15) | sc0;
	  unsigned int code27 = code30 >> 3;
	  unsigned int code3 = code30 & 7;
	  static const unsigned char pat3[] = {2, 3, 4, 5, 6, 9, 10, 14};
	  unsigned int pattern = pat3[code3];
	  unsigned int fused = (pattern << 27) | code27;

	  /* Move second reloc to same place as first.  */
	  irel_next->r_offset = irel->r_offset;

	  /* Change both relocs to R_FT32_NONE.  */

	  if (ELF32_R_TYPE (irel->r_info) == R_FT32_18)
	    {
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_FT32_SC0);
	    }
	  else
	    {
	      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
					   R_FT32_NONE);
	    }

	  if (ELF32_R_TYPE (irel_next->r_info) == R_FT32_18)
	    {
	      irel_next->r_info = ELF32_R_INFO (ELF32_R_SYM (irel_next->r_info),
						R_FT32_SC1);
	    }
	  else
	    {
	      irel_next->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
						R_FT32_NONE);
	    }

	  /* Replace the first insn with the fused version.  */
	  bfd_put_32 (abfd, fused, contents + irel->r_offset);

	  /* Delete the second insn.  */
	  if (!elf32_ft32_relax_delete_bytes (link_info, abfd, sec,
					       irel->r_offset + 4, 4))
	    goto error_return;

	  /* That will change things, so, we should relax again.
	     Note that this is not required, and it may be slow.  */
	  *again = true;

	  irel += 2;
	}
      else
	{
	  irel += 1;
	}
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
  free (free_relocs);

  return true;
}

#define ELF_ARCH		bfd_arch_ft32
#define ELF_MACHINE_CODE	EM_FT32
#define ELF_MAXPAGESIZE		0x1

#define TARGET_LITTLE_SYM       ft32_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-ft32"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			ft32_info_to_howto_rela
#define elf_backend_relocate_section		ft32_elf_relocate_section

#define elf_backend_can_gc_sections		1
#define elf_backend_rela_normal			1

#define bfd_elf32_bfd_reloc_type_lookup		ft32_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		ft32_reloc_name_lookup

#define bfd_elf32_bfd_relax_section		ft32_elf_relax_section

#include "elf32-target.h"
