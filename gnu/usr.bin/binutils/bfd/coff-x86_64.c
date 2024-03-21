/* BFD back-end for AMD 64 COFF files.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

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
   MA 02110-1301, USA.

   Written by Kai Tietz, OneVision Software GmbH&CoKg.  */

/* Note we have to make sure not to include headers twice.
   Not all headers are wrapped in #ifdef guards, so we define
   PEI_HEADERS to prevent double including here.  */
#ifndef PEI_HEADERS
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/x86_64.h"
#include "coff/internal.h"
#include "libcoff.h"
#include "libiberty.h"
#endif

#define BADMAG(x) AMD64BADMAG(x)

#ifdef COFF_WITH_pex64
# undef  AOUTSZ
# define AOUTSZ		PEPAOUTSZ
# define PEAOUTHDR	PEPAOUTHDR
#endif

#define COFF_DEFAULT_SECTION_ALIGNMENT_POWER (2)

/* The page size is a guess based on ELF.  */

#define COFF_PAGE_SIZE 0x1000

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

/* For some reason when using AMD COFF the value stored in the .text
   section for a reference to a common symbol is the value itself plus
   any desired offset.  Ian Taylor, Cygnus Support.  */

/* If we are producing relocatable output, we need to do some
   adjustments to the object file that are not done by the
   bfd_perform_relocation function.  This function is called by every
   reloc type to make any required adjustments.  */

static bfd_reloc_status_type
coff_amd64_reloc (bfd *abfd,
		  arelent *reloc_entry,
		  asymbol *symbol,
		  void * data,
		  asection *input_section ATTRIBUTE_UNUSED,
		  bfd *output_bfd,
		  char **error_message ATTRIBUTE_UNUSED)
{
  symvalue diff;

#if !defined (COFF_WITH_PE)
  if (output_bfd == NULL)
    return bfd_reloc_continue;
#endif

  if (bfd_is_com_section (symbol->section))
    {
#if !defined (COFF_WITH_PE)
      /* We are relocating a common symbol.  The current value in the
	 object file is ORIG + OFFSET, where ORIG is the value of the
	 common symbol as seen by the object file when it was compiled
	 (this may be zero if the symbol was undefined) and OFFSET is
	 the offset into the common symbol (normally zero, but may be
	 non-zero when referring to a field in a common structure).
	 ORIG is the negative of reloc_entry->addend, which is set by
	 the CALC_ADDEND macro below.  We want to replace the value in
	 the object file with NEW + OFFSET, where NEW is the value of
	 the common symbol which we are going to put in the final
	 object file.  NEW is symbol->value.  */
      diff = symbol->value + reloc_entry->addend;
#else
      /* In PE mode, we do not offset the common symbol.  */
      diff = reloc_entry->addend;
#endif
    }
  else
    {
      /* For some reason bfd_perform_relocation always effectively
	 ignores the addend for a COFF target when producing
	 relocatable output.  This seems to be always wrong for 386
	 COFF, so we handle the addend here instead.  */
#if defined (COFF_WITH_PE)
      if (output_bfd == NULL)
	{
	  if (symbol->flags & BSF_WEAK)
	    diff = reloc_entry->addend - symbol->value;
	  else
	    diff = -reloc_entry->addend;
	}
      else
#endif
	diff = reloc_entry->addend;
    }

#if defined (COFF_WITH_PE)
  if (output_bfd == NULL)
    {
      /* PC relative relocations are off by their size.  */
      if (reloc_entry->howto->pc_relative)
	diff -= bfd_get_reloc_size (reloc_entry->howto);

      if (reloc_entry->howto->type >= R_AMD64_PCRLONG_1
	  && reloc_entry->howto->type <= R_AMD64_PCRLONG_5)
	diff -= reloc_entry->howto->type - R_AMD64_PCRLONG;
    }

  if (reloc_entry->howto->type == R_AMD64_IMAGEBASE
      && output_bfd == NULL)
    {
      bfd *obfd = input_section->output_section->owner;
      struct bfd_link_info *link_info;
      struct bfd_link_hash_entry *h;
      switch (bfd_get_flavour (obfd))
	{
	case bfd_target_coff_flavour:
	  diff -= pe_data (obfd)->pe_opthdr.ImageBase;
	  break;
	case bfd_target_elf_flavour:
	  /* Subtract __ImageBase.  */
	  h = NULL;
	  link_info = _bfd_get_link_info (obfd);
	  if (link_info != NULL)
	    h = bfd_link_hash_lookup (link_info->hash, "__ImageBase",
				      false, false, true);
	  if (h == NULL
	      || (h->type != bfd_link_hash_defined
		  && h->type != bfd_link_hash_defweak))
	    {
	      *error_message
		= (char *) _("R_AMD64_IMAGEBASE with __ImageBase undefined");
	      return bfd_reloc_dangerous;
	    }
	  /* ELF symbols in relocatable files are section relative,
	     but in nonrelocatable files they are virtual addresses.  */
	  diff -= (h->u.def.value
		   + h->u.def.section->output_offset
		   + h->u.def.section->output_section->vma);
	  break;
	default:
	  break;
	}
    }
#endif

#define DOIT(x) \
  x = ((x & ~howto->dst_mask) | (((x & howto->src_mask) + diff) & howto->dst_mask))

  if (diff != 0)
    {
      reloc_howto_type *howto = reloc_entry->howto;
      bfd_size_type octets = (reloc_entry->address
			      * OCTETS_PER_BYTE (abfd, input_section));
      unsigned char *addr = (unsigned char *) data + octets;

      if (!bfd_reloc_offset_in_range (howto, abfd, input_section, octets))
	return bfd_reloc_outofrange;

      switch (bfd_get_reloc_size (howto))
	{
	case 1:
	  {
	    char x = bfd_get_8 (abfd, addr);
	    DOIT (x);
	    bfd_put_8 (abfd, x, addr);
	  }
	  break;

	case 2:
	  {
	    short x = bfd_get_16 (abfd, addr);
	    DOIT (x);
	    bfd_put_16 (abfd, (bfd_vma) x, addr);
	  }
	  break;

	case 4:
	  {
	    long x = bfd_get_32 (abfd, addr);
	    DOIT (x);
	    bfd_put_32 (abfd, (bfd_vma) x, addr);
	  }
	  break;

	case 8:
	  {
	    uint64_t x = bfd_get_64 (abfd, addr);
	    DOIT (x);
	    bfd_put_64 (abfd, x, addr);
	  }
	  break;

	default:
	  bfd_set_error (bfd_error_bad_value);
	  return bfd_reloc_notsupported;
	}
    }

  /* Now let bfd_perform_relocation finish everything up.  */
  return bfd_reloc_continue;
}

#if defined(COFF_WITH_PE)
/* Return TRUE if this relocation should appear in the output .reloc
   section.  */

static bool
in_reloc_p (bfd *abfd ATTRIBUTE_UNUSED, reloc_howto_type *howto)
{
  return ! howto->pc_relative
    && howto->type != R_AMD64_IMAGEBASE
    && howto->type != R_AMD64_SECREL
    && howto->type != R_AMD64_SECTION;
}
#endif /* COFF_WITH_PE */

#ifndef PCRELOFFSET
#define PCRELOFFSET true
#endif

static reloc_howto_type howto_table[] =
{
  EMPTY_HOWTO (0),
  HOWTO (R_AMD64_DIR64,		/* type  1*/
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_ADDR64", /* name */
	 true,			/* partial_inplace */
	 0xffffffffffffffffll,	/* src_mask */
	 0xffffffffffffffffll,	/* dst_mask */
	 true),			/* pcrel_offset */
  HOWTO (R_AMD64_DIR32,		/* type 2 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_ADDR32", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */
  /* PE IMAGE_REL_AMD64_ADDR32NB relocation (3).	*/
  HOWTO (R_AMD64_IMAGEBASE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_ADDR32NB", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
  /* 32-bit longword PC relative relocation (4).  */
  HOWTO (R_AMD64_PCRLONG,	/* type 4 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */

 HOWTO (R_AMD64_PCRLONG_1,	/* type 5 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32_1", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
 HOWTO (R_AMD64_PCRLONG_2,	/* type 6 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32_2", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
 HOWTO (R_AMD64_PCRLONG_3,	/* type 7 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32_3", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
 HOWTO (R_AMD64_PCRLONG_4,	/* type 8 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32_4", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
 HOWTO (R_AMD64_PCRLONG_5,	/* type 9 */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_REL32_5", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
#if defined(COFF_WITH_PE)
  /* 16-bit word section relocation (10).  */
  HOWTO (R_AMD64_SECTION,	/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_SECTION", /* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),
  /* 32-bit longword section relative relocation (11).  */
  HOWTO (R_AMD64_SECREL,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "IMAGE_REL_AMD64_SECREL", /* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */
#else
  EMPTY_HOWTO (10),
  EMPTY_HOWTO (11),
#endif
  EMPTY_HOWTO (12),
  EMPTY_HOWTO (13),
#ifndef DONT_EXTEND_AMD64
  HOWTO (R_AMD64_PCRQUAD,
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_PC64",	/* name */
	 true,			/* partial_inplace */
	 0xffffffffffffffffll,	/* src_mask */
	 0xffffffffffffffffll,	/* dst_mask */
	 PCRELOFFSET),		 /* pcrel_offset */
#else
  EMPTY_HOWTO (14),
#endif
  /* Byte relocation (15).  */
  HOWTO (R_RELBYTE,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_8",		/* name */
	 true,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
  /* 16-bit word relocation (16).  */
  HOWTO (R_RELWORD,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_16",		/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
  /* 32-bit longword relocation (17).	*/
  HOWTO (R_RELLONG,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_32S",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
  /* Byte PC relative relocation (18).	 */
  HOWTO (R_PCRBYTE,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_PC8",	/* name */
	 true,			/* partial_inplace */
	 0x000000ff,		/* src_mask */
	 0x000000ff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
  /* 16-bit word PC relative relocation (19).	*/
  HOWTO (R_PCRWORD,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_PC16",	/* name */
	 true,			/* partial_inplace */
	 0x0000ffff,		/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 PCRELOFFSET),		/* pcrel_offset */
  /* 32-bit longword PC relative relocation (20).  */
  HOWTO (R_PCRLONG,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 coff_amd64_reloc,	/* special_function */
	 "R_X86_64_PC32",	/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 PCRELOFFSET)		/* pcrel_offset */
};

#define NUM_HOWTOS ARRAY_SIZE (howto_table)

/* Turn a howto into a reloc  nunmber */

#define SELECT_RELOC(x,howto) { x.r_type = howto->type; }
#define I386  1			/* Customize coffcode.h */
#define AMD64 1

#define RTYPE2HOWTO(cache_ptr, dst)		\
  ((cache_ptr)->howto =				\
   ((dst)->r_type < NUM_HOWTOS)			\
    ? howto_table + (dst)->r_type		\
    : NULL)

/* For 386 COFF a STYP_NOLOAD | STYP_BSS section is part of a shared
   library.  On some other COFF targets STYP_BSS is normally
   STYP_NOLOAD.  */
#define BSS_NOLOAD_IS_SHARED_LIBRARY

/* Compute the addend of a reloc.  If the reloc is to a common symbol,
   the object file contains the value of the common symbol.  By the
   time this is called, the linker may be using a different symbol
   from a different object file with a different value.  Therefore, we
   hack wildly to locate the original symbol from this file so that we
   can make the correct adjustment.  This macro sets coffsym to the
   symbol from the original file, and uses it to set the addend value
   correctly.  If this is not a common symbol, the usual addend
   calculation is done, except that an additional tweak is needed for
   PC relative relocs.
   FIXME: This macro refers to symbols and asect; these are from the
   calling function, not the macro arguments.  */

#define CALC_ADDEND(abfd, ptr, reloc, cache_ptr)		\
  {								\
    coff_symbol_type *coffsym = NULL;				\
								\
    if (ptr && bfd_asymbol_bfd (ptr) != abfd)			\
      coffsym = (obj_symbols (abfd)				\
		 + (cache_ptr->sym_ptr_ptr - symbols));		\
    else if (ptr)						\
      coffsym = coff_symbol_from (ptr);				\
								\
    if (coffsym != NULL						\
	&& coffsym->native->u.syment.n_scnum == 0)		\
      cache_ptr->addend = - coffsym->native->u.syment.n_value;	\
    else if (ptr && bfd_asymbol_bfd (ptr) == abfd		\
	     && ptr->section != NULL)				\
      cache_ptr->addend = - (ptr->section->vma + ptr->value);	\
    else							\
      cache_ptr->addend = 0;					\
    if (ptr && reloc.r_type < NUM_HOWTOS			\
	&& howto_table[reloc.r_type].pc_relative)		\
      cache_ptr->addend += asect->vma;				\
  }

/* We use the special COFF backend linker.  For normal AMD64 COFF, we
   can use the generic relocate_section routine.  For PE, we need our
   own routine.  */

#if !defined(COFF_WITH_PE)

#define coff_relocate_section _bfd_coff_generic_relocate_section

#else /* COFF_WITH_PE */

/* The PE relocate section routine.  We handle secidx relocations here,
   as well as making sure that we don't do anything for a relocatable
   link.  */

static bool
coff_pe_amd64_relocate_section (bfd *output_bfd,
				struct bfd_link_info *info,
				bfd *input_bfd,
				asection *input_section,
				bfd_byte *contents,
				struct internal_reloc *relocs,
				struct internal_syment *syms,
				asection **sections)
{
  struct internal_reloc *rel;
  struct internal_reloc *relend;

  if (bfd_link_relocatable (info))
    return true;

  rel = relocs;
  relend = rel + input_section->reloc_count;

  for (; rel < relend; rel++)
    {
      long symndx;
      struct coff_link_hash_entry *h;
      asection *sec, *s;
      uint16_t idx = 0, i = 1;

      if (rel->r_type != R_SECTION)
	continue;

      /* Make sure that _bfd_coff_generic_relocate_section won't parse
         this reloc after us.  */
      rel->r_type = 0;

      symndx = rel->r_symndx;

      if (symndx < 0
	  || (unsigned long) symndx >= obj_raw_syment_count (input_bfd))
	continue;

      h = obj_coff_sym_hashes (input_bfd)[symndx];

      if (h == NULL)
	sec = sections[symndx];
      else
	{
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      /* Defined weak symbols are a GNU extension.  */
	      sec = h->root.u.def.section;
	    }
	  else
	    {
	      sec = NULL;
	    }
	}

      if (!sec)
	continue;

      if (bfd_is_abs_section (sec))
	continue;

      if (discarded_section (sec))
	continue;

      s = output_bfd->sections;
      while (s)
	{
	  if (s == sec->output_section)
	    {
	      idx = i;
	      break;
	    }

	  i++;
	  s = s->next;
	}

      bfd_putl16 (idx, contents + rel->r_vaddr - input_section->vma);
    }

  return _bfd_coff_generic_relocate_section (output_bfd, info, input_bfd,input_section, contents,relocs, syms, sections);
}

#define coff_relocate_section coff_pe_amd64_relocate_section

static hashval_t
htab_hash_section_index (const void * entry)
{
  const struct bfd_section * sec = entry;
  return sec->index;
}

static int
htab_eq_section_index (const void * e1, const void * e2)
{
  const struct bfd_section * sec1 = e1;
  const struct bfd_section * sec2 = e2;
  return sec1->index == sec2->index;
}
#endif /* COFF_WITH_PE */

/* Convert an rtype to howto for the COFF backend linker.  */

static reloc_howto_type *
coff_amd64_rtype_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			   asection *sec,
			   struct internal_reloc *rel,
			   struct coff_link_hash_entry *h,
			   struct internal_syment *sym,
			   bfd_vma *addendp)
{
  reloc_howto_type *howto;

  if (rel->r_type >= NUM_HOWTOS)
    {
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
  howto = howto_table + rel->r_type;

#if defined(COFF_WITH_PE)
  /* Cancel out code in _bfd_coff_generic_relocate_section.  */
  *addendp = 0;
  if (rel->r_type >= R_AMD64_PCRLONG_1 && rel->r_type <= R_AMD64_PCRLONG_5)
    {
      *addendp -= (bfd_vma)(rel->r_type - R_AMD64_PCRLONG);
      rel->r_type = R_AMD64_PCRLONG;
    }
#endif

  if (howto->pc_relative)
    *addendp += sec->vma;

  if (sym != NULL && sym->n_scnum == 0 && sym->n_value != 0)
    {
      /* This is a common symbol.  The section contents include the
	 size (sym->n_value) as an addend.  The relocate_section
	 function will be adding in the final value of the symbol.  We
	 need to subtract out the current size in order to get the
	 correct result.  */
      BFD_ASSERT (h != NULL);

#if !defined(COFF_WITH_PE)
      /* I think we *do* want to bypass this.  If we don't, I have
	 seen some data parameters get the wrong relocation address.
	 If I link two versions with and without this section bypassed
	 and then do a binary comparison, the addresses which are
	 different can be looked up in the map.  The case in which
	 this section has been bypassed has addresses which correspond
	 to values I can find in the map.  */
      *addendp -= sym->n_value;
#endif
    }

#if !defined(COFF_WITH_PE)
  /* If the output symbol is common (in which case this must be a
     relocatable link), we need to add in the final size of the
     common symbol.  */
  if (h != NULL && h->root.type == bfd_link_hash_common)
    *addendp += h->root.u.c.size;
#endif

#if defined(COFF_WITH_PE)
  if (howto->pc_relative)
    {
#ifndef DONT_EXTEND_AMD64
      if (rel->r_type == R_AMD64_PCRQUAD)
	*addendp -= 8;
      else
#endif
	*addendp -= 4;

      /* If the symbol is defined, then the generic code is going to
	 add back the symbol value in order to cancel out an
	 adjustment it made to the addend.  However, we set the addend
	 to 0 at the start of this function.  We need to adjust here,
	 to avoid the adjustment the generic code will make.  FIXME:
	 This is getting a bit hackish.  */
      if (sym != NULL && sym->n_scnum != 0)
	*addendp -= sym->n_value;
    }

  if (rel->r_type == R_AMD64_IMAGEBASE
      && (bfd_get_flavour (sec->output_section->owner) == bfd_target_coff_flavour))
    *addendp -= pe_data (sec->output_section->owner)->pe_opthdr.ImageBase;

  if (rel->r_type == R_AMD64_SECREL)
    {
      bfd_vma osect_vma = 0;

      if (h != NULL
	  && (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak))
	osect_vma = h->root.u.def.section->output_section->vma;
      else
	{
	  htab_t table = coff_data (abfd)->section_by_index;
	  asection *s;

	  if (!table)
	    {
	      table = htab_create (10, htab_hash_section_index,
				   htab_eq_section_index, NULL);
	      if (table == NULL)
		return NULL;
	      coff_data (abfd)->section_by_index = table;
	    }

	  if (htab_elements (table) == 0)
	    {
	      for (s = abfd->sections; s != NULL; s = s->next)
		{
		  void ** slot = htab_find_slot (table, s, INSERT);

		  if (slot != NULL)
		    *slot = s;
		}
	    }

	  struct bfd_section needle;

	  needle.index = sym->n_scnum - 1;
	  s = htab_find (table, &needle);
	  if (s != NULL)
	    osect_vma = s->output_section->vma;
	}

      *addendp -= osect_vma;
    }
#endif

  return howto;
}

#define coff_bfd_reloc_type_lookup coff_amd64_reloc_type_lookup
#define coff_bfd_reloc_name_lookup coff_amd64_reloc_name_lookup

static reloc_howto_type *
coff_amd64_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_RVA:
      return howto_table + R_AMD64_IMAGEBASE;
    case BFD_RELOC_32:
      return howto_table + R_AMD64_DIR32;
    case BFD_RELOC_64:
      return howto_table + R_AMD64_DIR64;
    case BFD_RELOC_64_PCREL:
#ifndef DONT_EXTEND_AMD64
      return howto_table + R_AMD64_PCRQUAD;
#else
      /* Fall through.  */
#endif
    case BFD_RELOC_32_PCREL:
      return howto_table + R_AMD64_PCRLONG;
    case BFD_RELOC_X86_64_32S:
      return howto_table + R_RELLONG;
    case BFD_RELOC_16:
      return howto_table + R_RELWORD;
    case BFD_RELOC_16_PCREL:
      return howto_table + R_PCRWORD;
    case BFD_RELOC_8:
      return howto_table + R_RELBYTE;
    case BFD_RELOC_8_PCREL:
      return howto_table + R_PCRBYTE;
#if defined(COFF_WITH_PE)
    case BFD_RELOC_32_SECREL:
      return howto_table + R_AMD64_SECREL;
    case BFD_RELOC_16_SECIDX:
      return howto_table + R_AMD64_SECTION;
#endif
    default:
      BFD_FAIL ();
      return 0;
    }
}

static reloc_howto_type *
coff_amd64_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *r_name)
{
  unsigned int i;

  for (i = 0; i < NUM_HOWTOS; i++)
    if (howto_table[i].name != NULL
	&& strcasecmp (howto_table[i].name, r_name) == 0)
      return &howto_table[i];

  return NULL;
}

#define coff_rtype_to_howto coff_amd64_rtype_to_howto

#ifdef TARGET_UNDERSCORE

/* If amd64 gcc uses underscores for symbol names, then it does not use
   a leading dot for local labels, so if TARGET_UNDERSCORE is defined
   we treat all symbols starting with L as local.  */

static bool
coff_amd64_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == 'L')
    return true;

  return _bfd_coff_is_local_label_name (abfd, name);
}

#define coff_bfd_is_local_label_name coff_amd64_is_local_label_name

#endif /* TARGET_UNDERSCORE */

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata   NULL
#endif

#include "coffcode.h"

#ifdef PE
#define amd64coff_object_p pe_bfd_object_p
#else
#define amd64coff_object_p coff_object_p
#endif

const bfd_target
#ifdef TARGET_SYM
  TARGET_SYM =
#else
  x86_64_coff_vec =
#endif
{
#ifdef TARGET_NAME
  TARGET_NAME,
#else
 "coff-x86-64",			/* Name.  */
#endif
  bfd_target_coff_flavour,
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */
  BFD_ENDIAN_LITTLE,		/* Header byte order is little.  */

  (HAS_RELOC | EXEC_P		/* Object flags.  */
   | HAS_LINENO | HAS_DEBUG
   | HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED | BFD_COMPRESS | BFD_DECOMPRESS),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC /* Section flags.  */
#if defined(COFF_WITH_PE)
   | SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_READONLY | SEC_DEBUGGING
#endif
   | SEC_CODE | SEC_DATA | SEC_EXCLUDE ),

#ifdef TARGET_UNDERSCORE
  TARGET_UNDERSCORE,		/* Leading underscore.  */
#else
  0,				/* Leading underscore.  */
#endif
  '/',				/* Ar_pad_char.  */
  15,				/* Ar_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Data.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Hdrs.  */

  /* Note that we allow an object file to be treated as a core file as well.  */
  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    amd64coff_object_p,
    bfd_generic_archive_p,
    amd64coff_object_p
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    coff_mkobject,
    _bfd_generic_mkarchive,
    _bfd_bool_bfd_false_error
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    coff_write_object_contents,
    _bfd_write_archive_contents,
    _bfd_bool_bfd_false_error
  },

  BFD_JUMP_TABLE_GENERIC (coff),
  BFD_JUMP_TABLE_COPY (coff),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),
  BFD_JUMP_TABLE_SYMBOLS (coff),
  BFD_JUMP_TABLE_RELOCS (coff),
  BFD_JUMP_TABLE_WRITE (coff),
  BFD_JUMP_TABLE_LINK (coff),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  COFF_SWAP_TABLE
};

/* Entry for big object files.  */

#ifdef COFF_WITH_PE_BIGOBJ
const bfd_target
  TARGET_SYM_BIG =
{
  TARGET_NAME_BIG,
  bfd_target_coff_flavour,
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */
  BFD_ENDIAN_LITTLE,		/* Header byte order is little.  */

  (HAS_RELOC | EXEC_P		/* Object flags.  */
   | HAS_LINENO | HAS_DEBUG
   | HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED | BFD_COMPRESS | BFD_DECOMPRESS),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC /* Section flags.  */
#if defined(COFF_WITH_PE)
   | SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_READONLY | SEC_DEBUGGING
#endif
   | SEC_CODE | SEC_DATA | SEC_EXCLUDE ),

#ifdef TARGET_UNDERSCORE
  TARGET_UNDERSCORE,		/* Leading underscore.  */
#else
  0,				/* Leading underscore.  */
#endif
  '/',				/* Ar_pad_char.  */
  15,				/* Ar_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Data.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Hdrs.  */

  /* Note that we allow an object file to be treated as a core file as well.  */
  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    amd64coff_object_p,
    bfd_generic_archive_p,
    amd64coff_object_p
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    coff_mkobject,
    _bfd_generic_mkarchive,
    _bfd_bool_bfd_false_error
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    coff_write_object_contents,
    _bfd_write_archive_contents,
    _bfd_bool_bfd_false_error
  },

  BFD_JUMP_TABLE_GENERIC (coff),
  BFD_JUMP_TABLE_COPY (coff),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),
  BFD_JUMP_TABLE_SYMBOLS (coff),
  BFD_JUMP_TABLE_RELOCS (coff),
  BFD_JUMP_TABLE_WRITE (coff),
  BFD_JUMP_TABLE_LINK (coff),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  &bigobj_swap_table
};
#endif
