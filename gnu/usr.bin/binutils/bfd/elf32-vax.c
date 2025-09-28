/* VAX series support for 32-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   Contributed by Matt Thomas <matt@3am-software.com>.

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
#include "elf/vax.h"

static reloc_howto_type *reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static bool rtype_to_howto (bfd *, arelent *, Elf_Internal_Rela *);
static struct bfd_hash_entry *elf_vax_link_hash_newfunc (struct bfd_hash_entry *,
							 struct bfd_hash_table *,
							 const char *);
static struct bfd_link_hash_table *elf_vax_link_hash_table_create (bfd *);
static bool elf_vax_check_relocs (bfd *, struct bfd_link_info *,
				  asection *, const Elf_Internal_Rela *);
static bool elf_vax_adjust_dynamic_symbol (struct bfd_link_info *,
					   struct elf_link_hash_entry *);
static bool elf_vax_size_dynamic_sections (bfd *, struct bfd_link_info *);
static int elf_vax_relocate_section (bfd *, struct bfd_link_info *,
				     bfd *, asection *, bfd_byte *,
				     Elf_Internal_Rela *,
				     Elf_Internal_Sym *, asection **);
static bool elf_vax_finish_dynamic_symbol (bfd *, struct bfd_link_info *,
					   struct elf_link_hash_entry *,
					   Elf_Internal_Sym *);
static bool elf_vax_finish_dynamic_sections (bfd *, struct bfd_link_info *);
static bfd_vma elf_vax_plt_sym_val (bfd_vma, const asection *,
				    const arelent *);

static bool elf32_vax_set_private_flags (bfd *, flagword);
static bool elf32_vax_print_private_bfd_data (bfd *, void *);

static reloc_howto_type howto_table[] = {
  HOWTO (R_VAX_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x00000000,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_PC32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_PC32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_VAX_PC16,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_PC16",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_VAX_PC8,		/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 8,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_PC8",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x000000ff,		/* dst_mask */
	 true),			/* pcrel_offset */

  HOWTO (R_VAX_GOT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_GOT32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),

  HOWTO (R_VAX_PLT32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_PLT32",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 true),			/* pcrel_offset */

  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),
  EMPTY_HOWTO (-1),

  HOWTO (R_VAX_COPY,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_COPY",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_GLOB_DAT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_GLOB_DAT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_JMP_SLOT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_JMP_SLOT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO (R_VAX_RELATIVE,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_VAX_RELATIVE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable hierarchy */
  HOWTO (R_VAX_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_VAX_GNU_VTINHERIT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* GNU extension to record C++ vtable member usage */
  HOWTO (R_VAX_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_VAX_GNU_VTENTRY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */
};

static bool
rtype_to_howto (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= R_VAX_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &howto_table[r_type];
  return true;
}

#define elf_info_to_howto rtype_to_howto

static const struct
{
  bfd_reloc_code_real_type bfd_val;
  int elf_val;
} reloc_map[] = {
  { BFD_RELOC_NONE, R_VAX_NONE },
  { BFD_RELOC_32, R_VAX_32 },
  { BFD_RELOC_16, R_VAX_16 },
  { BFD_RELOC_8, R_VAX_8 },
  { BFD_RELOC_32_PCREL, R_VAX_PC32 },
  { BFD_RELOC_16_PCREL, R_VAX_PC16 },
  { BFD_RELOC_8_PCREL, R_VAX_PC8 },
  { BFD_RELOC_32_GOT_PCREL, R_VAX_GOT32 },
  { BFD_RELOC_32_PLT_PCREL, R_VAX_PLT32 },
  { BFD_RELOC_NONE, R_VAX_COPY },
  { BFD_RELOC_VAX_GLOB_DAT, R_VAX_GLOB_DAT },
  { BFD_RELOC_VAX_JMP_SLOT, R_VAX_JMP_SLOT },
  { BFD_RELOC_VAX_RELATIVE, R_VAX_RELATIVE },
  { BFD_RELOC_CTOR, R_VAX_32 },
  { BFD_RELOC_VTABLE_INHERIT, R_VAX_GNU_VTINHERIT },
  { BFD_RELOC_VTABLE_ENTRY, R_VAX_GNU_VTENTRY },
};

static reloc_howto_type *
reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  unsigned int i;
  for (i = 0; i < sizeof (reloc_map) / sizeof (reloc_map[0]); i++)
    {
      if (reloc_map[i].bfd_val == code)
	return &howto_table[reloc_map[i].elf_val];
    }
  return 0;
}

static reloc_howto_type *
reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
		   const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (howto_table) / sizeof (howto_table[0]); i++)
    if (howto_table[i].name != NULL
	&& strcasecmp (howto_table[i].name, r_name) == 0)
      return &howto_table[i];

  return NULL;
}

#define bfd_elf32_bfd_reloc_type_lookup reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup reloc_name_lookup
#define ELF_ARCH bfd_arch_vax
/* end code generated by elf.el */

/* Functions for the VAX ELF linker.  */

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/usr/libexec/ld.elf_so"

/* The size in bytes of an entry in the procedure linkage table.  */

#define PLT_ENTRY_SIZE 12

/* The first entry in a procedure linkage table looks like this.  See
   the SVR4 ABI VAX supplement to see how this works.  */

static const bfd_byte elf_vax_plt0_entry[PLT_ENTRY_SIZE] =
{
  0xdd, 0xef,		/* pushl l^ */
  0, 0, 0, 0,		/* offset to .plt.got + 4 */
  0x17, 0xff,		/* jmp @L^(pc) */
  0, 0, 0, 0,		/* offset to .plt.got + 8 */
};

/* Subsequent entries in a procedure linkage table look like this.  */

static const bfd_byte elf_vax_plt_entry[PLT_ENTRY_SIZE] =
{
  0xfc, 0x0f,		/* .word ^M<r11:r2> */
  0x16, 0xef,		/* jsb L^(pc) */
  0, 0, 0, 0,		/* replaced with offset to start of .plt  */
  0, 0, 0, 0,		/* index into .rela.plt */
};

/* The VAX linker needs to keep track of the number of relocs that it
   decides to copy in check_relocs for each symbol.  This is so that it
   can discard PC relative relocs if it doesn't need them when linking
   with -Bsymbolic.  We store the information in a field extending the
   regular ELF linker hash table.  */

/* This structure keeps track of the number of PC relative relocs we have
   copied for a given symbol.  */

struct elf_vax_pcrel_relocs_copied
{
  /* Next section.  */
  struct elf_vax_pcrel_relocs_copied *next;
  /* A section in dynobj.  */
  asection *section;
  /* Number of relocs copied in this section.  */
  bfd_size_type count;
};

/* VAX ELF linker hash entry.  */

struct elf_vax_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Number of PC relative relocs copied for this symbol.  */
  struct elf_vax_pcrel_relocs_copied *pcrel_relocs_copied;

  bfd_vma got_addend;
};

/* Declare this now that the above structures are defined.  */

static bool elf_vax_discard_copies (struct elf_vax_link_hash_entry *,
				    void *);

/* Declare this now that the above structures are defined.  */

static bool elf_vax_instantiate_got_entries (struct elf_link_hash_entry *,
					     void *);

/* Traverse an VAX ELF linker hash table.  */

#define elf_vax_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   ((table),								\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),		\
    (info)))

/* Create an entry in an VAX ELF linker hash table.  */

static struct bfd_hash_entry *
elf_vax_link_hash_newfunc (struct bfd_hash_entry *entry,
			   struct bfd_hash_table *table,
			   const char *string)
{
  struct elf_vax_link_hash_entry *ret =
    (struct elf_vax_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = ((struct elf_vax_link_hash_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct elf_vax_link_hash_entry)));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_vax_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      ret->pcrel_relocs_copied = NULL;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an VAX ELF linker hash table.  */

static struct bfd_link_hash_table *
elf_vax_link_hash_table_create (bfd *abfd)
{
  struct elf_link_hash_table *ret;
  size_t amt = sizeof (struct elf_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (ret, abfd,
				      elf_vax_link_hash_newfunc,
				      sizeof (struct elf_vax_link_hash_entry),
				      GENERIC_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->root;
}

/* Keep vax-specific flags in the ELF header */
static bool
elf32_vax_set_private_flags (bfd *abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */
static bool
elf32_vax_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  flagword in_flags;

  if (   bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  in_flags  = elf_elfheader (ibfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = in_flags;
    }

  return true;
}

/* Display the flags field */
static bool
elf32_vax_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* Ignore init flag - it may not be set, despite the flags field containing valid data.  */

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (elf_elfheader (abfd)->e_flags & EF_VAX_NONPIC)
    fprintf (file, _(" [nonpic]"));

  if (elf_elfheader (abfd)->e_flags & EF_VAX_DFLOAT)
    fprintf (file, _(" [d-float]"));

  if (elf_elfheader (abfd)->e_flags & EF_VAX_GFLOAT)
    fprintf (file, _(" [g-float]"));

  fputc ('\n', file);

  return true;
}
/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bool
elf_vax_check_relocs (bfd *abfd, struct bfd_link_info *info, asection *sec,
		      const Elf_Internal_Rela *relocs)
{
  bfd *dynobj;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;

  if (bfd_link_relocatable (info))
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  sreloc = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;

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
	case R_VAX_GOT32:
	  BFD_ASSERT (h != NULL);

	  /* If this is a local symbol, we resolve it directly without
	     creating a global offset table entry.  */
	  if (SYMBOL_REFERENCES_LOCAL (info, h)
	      || h == elf_hash_table (info)->hgot
	      || h == elf_hash_table (info)->hplt)
	    break;

	  /* This symbol requires a global offset table entry.  */

	  if (dynobj == NULL)
	    {
	      /* Create the .got section.  */
	      elf_hash_table (info)->dynobj = dynobj = abfd;
	      if (!_bfd_elf_create_got_section (dynobj, info))
		return false;
	    }

	  if (h != NULL)
	    {
	      struct elf_vax_link_hash_entry *eh;

	      eh = (struct elf_vax_link_hash_entry *) h;
	      if (h->got.refcount == -1)
		{
		  h->got.refcount = 1;
		  eh->got_addend = rel->r_addend;
		}
	      else
		{
		  h->got.refcount++;
		  if (eh->got_addend != (bfd_vma) rel->r_addend)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: warning: GOT addend of %" PRId64 " to `%s' does"
			 " not match previous GOT addend of %" PRId64),
			 abfd, (int64_t) rel->r_addend, h->root.root.string,
			 (int64_t) eh->got_addend);

		}
	    }
	  break;

	case R_VAX_PLT32:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code which is
	     never referenced by a dynamic object, in which case we
	     don't need to generate a procedure linkage table entry
	     after all.  */
	  BFD_ASSERT (h != NULL);

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (h->forced_local)
	    break;

	  h->needs_plt = 1;
	  if (h->plt.refcount == -1)
	    h->plt.refcount = 1;
	  else
	    h->plt.refcount++;
	  break;

	case R_VAX_PC8:
	case R_VAX_PC16:
	case R_VAX_PC32:
	  /* If we are creating a shared library and this is not a local
	     symbol, we need to copy the reloc into the shared library.
	     However when linking with -Bsymbolic and this is a global
	     symbol which is defined in an object we are including in the
	     link (i.e., DEF_REGULAR is set), then we can resolve the
	     reloc directly.  At this point we have not seen all the input
	     files, so it is possible that DEF_REGULAR is not set now but
	     will be set later (it is never cleared).  We account for that
	     possibility below by storing information in the
	     pcrel_relocs_copied field of the hash table entry.  */
	  if (!(bfd_link_pic (info)
		&& (sec->flags & SEC_ALLOC) != 0
		&& h != NULL
		&& (!info->symbolic
		    || !h->def_regular)))
	    {
	      if (h != NULL
		  && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		  && !h->forced_local)
		{
		  /* Make sure a plt entry is created for this symbol if
		     it turns out to be a function defined by a dynamic
		     object.  */
		  if (h->plt.refcount == -1)
		    h->plt.refcount = 1;
		  else
		    h->plt.refcount++;
		}
	      break;
	    }
	  /* If this is a local symbol, we can resolve it directly.  */
	  if (h != NULL
	      && (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		  || h->forced_local))
	    break;

	  /* Fall through.  */
	case R_VAX_8:
	case R_VAX_16:
	case R_VAX_32:
	  if (h != NULL && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
	    {
	      /* Make sure a plt entry is created for this symbol if it
		 turns out to be a function defined by a dynamic object.  */
	      if (h->plt.refcount == -1)
		h->plt.refcount = 1;
	      else
		h->plt.refcount++;
	    }

	  /* Non-GOT reference may need a copy reloc in executable or
	     a dynamic reloc in shared library.  */
	  if (h != NULL)
	    h->non_got_ref = 1;

	  /* If we are creating a shared library, we need to copy the
	     reloc into the shared library.  */
	  if (bfd_link_pic (info)
	      && (sec->flags & SEC_ALLOC) != 0)
	    {
	      /* When creating a shared object, we must copy these
		 reloc types into the output file.  We create a reloc
		 section in dynobj and make room for this reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, dynobj, 2, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    return false;

		  if (sec->flags & SEC_READONLY)
		    info->flags |= DF_TEXTREL;
		}

	      sreloc->size += sizeof (Elf32_External_Rela);

	      /* If we are linking with -Bsymbolic, we count the number of
		 PC relative relocations we have entered for this symbol,
		 so that we can discard them again if the symbol is later
		 defined by a regular object.  Note that this function is
		 only called if we are using a vaxelf linker hash table,
		 which means that h is really a pointer to an
		 elf_vax_link_hash_entry.  */
	      if ((ELF32_R_TYPE (rel->r_info) == R_VAX_PC8
		   || ELF32_R_TYPE (rel->r_info) == R_VAX_PC16
		   || ELF32_R_TYPE (rel->r_info) == R_VAX_PC32)
		  && info->symbolic)
		{
		  struct elf_vax_link_hash_entry *eh;
		  struct elf_vax_pcrel_relocs_copied *p;

		  eh = (struct elf_vax_link_hash_entry *) h;

		  for (p = eh->pcrel_relocs_copied; p != NULL; p = p->next)
		    if (p->section == sreloc)
		      break;

		  if (p == NULL)
		    {
		      p = ((struct elf_vax_pcrel_relocs_copied *)
			   bfd_alloc (dynobj, (bfd_size_type) sizeof *p));
		      if (p == NULL)
			return false;
		      p->next = eh->pcrel_relocs_copied;
		      eh->pcrel_relocs_copied = p;
		      p->section = sreloc;
		      p->count = 0;
		    }

		  ++p->count;
		}
	    }

	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_VAX_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_VAX_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
elf_vax_gc_mark_hook (asection *sec,
		      struct bfd_link_info *info,
		      Elf_Internal_Rela *rel,
		      struct elf_link_hash_entry *h,
		      Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_VAX_GNU_VTINHERIT:
      case R_VAX_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf_vax_adjust_dynamic_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *h)
{
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
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLTxx reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PCxx reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	  return true;
	}

      s = elf_hash_table (info)->splt;
      BFD_ASSERT (s != NULL);

      /* If this is the first .plt entry, make room for the special
	 first entry.  */
      if (s->size == 0)
	{
	  s->size += PLT_ENTRY_SIZE;
	}

      /* If this symbol is not defined in a regular file, and we are
	 not generating a shared library, then set the symbol to this
	 location in the .plt.  This is required to make function
	 pointers compare as equal between the normal executable and
	 the shared library.  */
      if (!bfd_link_pic (info)
	  && !h->def_regular)
	{
	  h->root.u.def.section = s;
	  h->root.u.def.value = s->size;
	}

      h->plt.offset = s->size;

      /* Make room for this entry.  */
      s->size += PLT_ENTRY_SIZE;

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

  /* Reinitialize the plt offset now that it is not used as a reference
     count any more.  */
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
     GOT relocation, we don't need to generate a copy reloc.  */
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

  /* We must generate a R_VAX_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      srel = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* This function is called via elf_link_hash_traverse.  It resets GOT
   and PLT (.GOT) reference counts back to -1 so normal PC32 relocation
   will be done.  */

static bool
elf_vax_discard_got_entries (struct elf_link_hash_entry *h,
			     void *infoptr ATTRIBUTE_UNUSED)
{
  h->got.refcount = -1;
  h->plt.refcount = -1;

  return true;
}

/* Discard unused dynamic data if this is a static link.  */

static bool
elf_vax_always_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;

  dynobj = elf_hash_table (info)->dynobj;

  if (dynobj && !elf_hash_table (info)->dynamic_sections_created)
    {
      /* We may have created entries in the .rela.got and .got sections.
	 However, if we are not creating the dynamic sections, we will
	 not actually use these entries.  Reset the size of .rela.got
	 and .got, which will cause them to get stripped from the output
	 file below.  */
      s = elf_hash_table (info)->srelgot;
      if (s != NULL)
	s->size = 0;
      s = elf_hash_table (info)->sgotplt;
      if (s != NULL)
	s->size = 0;
      s = elf_hash_table (info)->sgot;
      if (s != NULL)
	s->size = 0;
    }

  /* If this is a static link, we need to discard all the got entries we've
     recorded.  */
  if (!dynobj || !elf_hash_table (info)->dynamic_sections_created)
    elf_link_hash_traverse (elf_hash_table (info),
			    elf_vax_discard_got_entries,
			    info);

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
elf_vax_size_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bool relocs;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
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

  /* If this is a -Bsymbolic shared link, then we need to discard all PC
     relative relocs against symbols defined in a regular object.  We
     allocated space for them in the check_relocs routine, but we will not
     fill them in in the relocate_section routine.  */
  if (bfd_link_pic (info) && info->symbolic)
    elf_vax_link_hash_traverse (elf_hash_table (info),
				elf_vax_discard_copies,
				NULL);

  /* If this is a -Bsymbolic shared link, we need to discard all the got
     entries we've recorded.  Otherwise, we need to instantiate (allocate
     space for them).  */
  elf_link_hash_traverse (elf_hash_table (info),
			  elf_vax_instantiate_got_entries,
			  info);

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;

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
	      if (strcmp (name, ".rela.plt") != 0)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else if (! startswith (name, ".got")
	       && strcmp (name, ".dynbss") != 0)
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

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

      /* Allocate memory for the section contents.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* This function is called via elf_vax_link_hash_traverse if we are
   creating a shared object with -Bsymbolic.  It discards the space
   allocated to copy PC relative relocs against symbols which are defined
   in regular objects.  We allocated space for them in the check_relocs
   routine, but we won't fill them in in the relocate_section routine.  */

static bool
elf_vax_discard_copies (struct elf_vax_link_hash_entry *h,
			void * ignore ATTRIBUTE_UNUSED)
{
  struct elf_vax_pcrel_relocs_copied *s;

  /* We only discard relocs for symbols defined in a regular object.  */
  if (!h->root.def_regular)
    return true;

  for (s = h->pcrel_relocs_copied; s != NULL; s = s->next)
    s->section->size -= s->count * sizeof (Elf32_External_Rela);

  return true;
}

/* This function is called via elf_link_hash_traverse.  It looks for
   entries that have GOT or PLT (.GOT) references.  If creating a shared
   object with -Bsymbolic, or the symbol has been forced local, then it
   resets the reference count back to -1 so normal PC32 relocation will
   be done.  Otherwise space in the .got and .rela.got will be reserved
   for the symbol.  */

static bool
elf_vax_instantiate_got_entries (struct elf_link_hash_entry *h, void * infoptr)
{
  struct bfd_link_info *info = (struct bfd_link_info *) infoptr;
  bfd *dynobj;
  asection *sgot;
  asection *srelgot;

  /* We don't care about non-GOT (and non-PLT) entries.  */
  if (h->got.refcount <= 0 && h->plt.refcount <= 0)
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  sgot = elf_hash_table (info)->sgot;
  srelgot = elf_hash_table (info)->srelgot;

  if (SYMBOL_REFERENCES_LOCAL (info, h))
    {
      h->got.refcount = -1;
      h->plt.refcount = -1;
    }
  else if (h->got.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.  */
      if (h->dynindx == -1)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      /* Allocate space in the .got and .rela.got sections.  */
      sgot->size += 4;
      srelgot->size += sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Relocate an VAX ELF section.  */

static int
elf_vax_relocate_section (bfd *output_bfd,
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
  bfd_vma plt_index;
  bfd_vma got_offset;
  asection *sgot;
  asection *splt;
  asection *sgotplt;
  asection *sreloc;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  sgot = NULL;
  splt = NULL;
  sgotplt = NULL;
  sreloc = NULL;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma relocation;
      bfd_reloc_status_type r;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type < 0 || r_type >= (int) R_VAX_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      howto = howto_table + r_type;

      r_symndx = ELF32_R_SYM (rel->r_info);
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
	  bool unresolved_reloc;
	  bool warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  if ((h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	      && ((r_type == R_VAX_PLT32
		   && h->plt.offset != (bfd_vma) -1
		   && !h->forced_local
		   && elf_hash_table (info)->dynamic_sections_created)
		  || (r_type == R_VAX_GOT32
		      && h->got.offset != (bfd_vma) -1
		      && !h->forced_local
		      && elf_hash_table (info)->dynamic_sections_created
		      && (! bfd_link_pic (info)
			  || (! info->symbolic && h->dynindx != -1)
			  || !h->def_regular))
		  || (bfd_link_pic (info)
		      && ((! info->symbolic && h->dynindx != -1)
			  || !h->def_regular)
		      && ((input_section->flags & SEC_ALLOC) != 0
			  /* DWARF will emit R_VAX_32 relocations in its
			     sections against symbols defined externally
			     in shared libraries.  We can't do anything
			     with them here.  */

			  || ((input_section->flags & SEC_DEBUGGING) != 0
			      && h->def_dynamic))
		      && (r_type == R_VAX_8
			  || r_type == R_VAX_16
			  || r_type == R_VAX_32))))
	    /* In these cases, we don't need the relocation
	       value.  We check specially because in some
	       obscure cases sec->output_section will be NULL.  */
	    relocation = 0;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      switch (r_type)
	{
	case R_VAX_GOT32:
	  /* Relocation is to the address of the entry for this symbol
	     in the global offset table.  */

	  /* Resolve a GOTxx reloc against a local symbol directly,
	     without using the global offset table.  */
	  if (h == NULL
	      || h->got.offset == (bfd_vma) -1)
	    break;

	  {
	    bfd_vma off;

	    sgot = elf_hash_table (info)->sgot;
	    BFD_ASSERT (sgot != NULL);

	    off = h->got.offset;
	    BFD_ASSERT (off < sgot->size);

	    bfd_put_32 (output_bfd, rel->r_addend, sgot->contents + off);

	    relocation = sgot->output_offset + off;
	    /* The GOT relocation uses the addend.  */
	    rel->r_addend = 0;

	    /* Change the reference to be indirect.  */
	    contents[rel->r_offset - 1] |= 0x10;
	    relocation += sgot->output_section->vma;
	  }
	  break;

	case R_VAX_PC32:
	  /* If we are creating an executable and the function this
	     reloc refers to is in a shared lib, then we made a PLT
	     entry for this symbol and need to handle the reloc like
	     a PLT reloc.  */
	  if (bfd_link_pic (info))
	     goto r_vax_pc32_shared;
	  /* Fall through.  */
	case R_VAX_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLTxx reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL
	      || h->plt.offset == (bfd_vma) -1)
	    break;

	  splt = elf_hash_table (info)->splt;
	  BFD_ASSERT (splt != NULL);

	  sgotplt = elf_hash_table (info)->sgotplt;
	  BFD_ASSERT (sgotplt != NULL);

	  plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

	  /* Get the offset into the .got table of the entry that
	     corresponds to this function.  Each .got entry is 4 bytes.
	     The first two are reserved.  */
	  got_offset = (plt_index + 3) * 4;

	  /* We want the relocation to point into the .got.plt instead
	     of the plt itself.  */
	  relocation = (sgotplt->output_section->vma
			+ sgotplt->output_offset
			+ got_offset);
	  contents[rel->r_offset-1] |= 0x10; /* make indirect */
	  if (rel->r_addend == 2)
	    {
	      h->plt.offset |= 1;
	    }
	  else if (rel->r_addend != 0)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: warning: PLT addend of %" PRId64 " to `%s'"
		 " from %pA section ignored"),
	       input_bfd, (int64_t) rel->r_addend, h->root.root.string,
	       input_section);
	  rel->r_addend = 0;

	  break;

	case R_VAX_PC8:
	case R_VAX_PC16:
	r_vax_pc32_shared:
	  if (h == NULL
	      || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      || h->forced_local)
	    break;
	  /* Fall through.  */
	case R_VAX_8:
	case R_VAX_16:
	case R_VAX_32:
	  if (bfd_link_pic (info)
	      && r_symndx != STN_UNDEF
	      && (input_section->flags & SEC_ALLOC) != 0
	      && ((r_type != R_VAX_PC8
		   && r_type != R_VAX_PC16
		   && r_type != R_VAX_PC32)
		  || ((input_section->flags & SEC_CODE)
		      && (!info->symbolic
			  || (!h->def_regular && h->type != STT_SECTION)))))
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      bool skip, relocate;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_get_dynamic_reloc_section
		    (input_bfd, input_section, /*rela?*/ true);
		  if (sreloc == NULL)
		    return false;
		}

	      skip = false;
	      relocate = false;

	      outrel.r_offset =
		_bfd_elf_section_offset (output_bfd, info, input_section,
					 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1)
		skip = true;
	      if (outrel.r_offset == (bfd_vma) -2)
		skip = true, relocate = true;
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
		  outrel.r_addend = relocation + rel->r_addend;
		}
	      else
		{
		  if (r_type == R_VAX_32)
		    {
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_VAX_RELATIVE);
		      BFD_ASSERT (bfd_get_signed_32 (input_bfd,
						     &contents[rel->r_offset]) == 0);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      long indx;

		      if (bfd_is_abs_section (sec))
			indx = 0;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return false;
			}
		      else
			{
			  asection *osec;

			  /* We are turning this relocation into one
			     against a section symbol.  It would be
			     proper to subtract the symbol's value,
			     osec->vma, from the emitted reloc addend,
			     but ld.so expects buggy relocs.  */
			  osec = sec->output_section;
			  indx = elf_section_data (osec)->dynindx;
			  if (indx == 0)
			    {
			      struct elf_link_hash_table *htab;
			      htab = elf_hash_table (info);
			      osec = htab->text_index_section;
			      indx = elf_section_data (osec)->dynindx;
			    }
			  BFD_ASSERT (indx != 0);
			}

		      outrel.r_info = ELF32_R_INFO (indx, r_type);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		}

	      if ((input_section->flags & SEC_CODE) != 0
		  || (ELF32_R_TYPE (outrel.r_info) != R_VAX_32
		      && ELF32_R_TYPE (outrel.r_info) != R_VAX_RELATIVE
		      && ELF32_R_TYPE (outrel.r_info) != R_VAX_COPY
		      && ELF32_R_TYPE (outrel.r_info) != R_VAX_JMP_SLOT
		      && ELF32_R_TYPE (outrel.r_info) != R_VAX_GLOB_DAT))
		{
		  if (h != NULL)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: warning: %s relocation against symbol `%s'"
			 " from %pA section"),
		      input_bfd, howto->name, h->root.root.string,
		      input_section);
		  else
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: warning: %s relocation to %#" PRIx64
			 " from %pA section"),
		      input_bfd, howto->name, (uint64_t) outrel.r_addend,
		      input_section);
		}
	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      /* This reloc will be computed at runtime, so there's no
		 need to do anything now, except for R_VAX_32
		 relocations that have been turned into
		 R_VAX_RELATIVE.  */
	      if (!relocate)
		continue;
	    }

	  break;

	case R_VAX_GNU_VTINHERIT:
	case R_VAX_GNU_VTENTRY:
	  /* These are no-ops in the end.  */
	  continue;

	default:
	  break;
	}

      /* VAX PCREL relocations are from the end of relocation, not the start.
	 So subtract the difference from the relocation amount since we can't
	 add it to the offset.  */
      if (howto->pc_relative && howto->pcrel_offset)
	relocation -= bfd_get_reloc_size(howto);

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    default:
	    case bfd_reloc_outofrange:
	      abort ();
	    case bfd_reloc_overflow:
	      {
		const char *name;

		if (h != NULL)
		  name = NULL;
		else
		  {
		    name = bfd_elf_string_from_elf_section (input_bfd,
							    symtab_hdr->sh_link,
							    sym->st_name);
		    if (name == NULL)
		      return false;
		    if (*name == '\0')
		      name = bfd_section_name (sec);
		  }
		info->callbacks->reloc_overflow
		  (info, (h ? &h->root : NULL), name, howto->name,
		   (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      }
	      break;
	    }
	}
    }

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf_vax_finish_dynamic_symbol (bfd *output_bfd, struct bfd_link_info *info,
			       struct elf_link_hash_entry *h,
			       Elf_Internal_Sym *sym)
{
  bfd *dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgot;
      asection *srela;
      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma addend;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = elf_hash_table (info)->splt;
      sgot = elf_hash_table (info)->sgotplt;
      srela = elf_hash_table (info)->srelplt;
      BFD_ASSERT (splt != NULL && sgot != NULL && srela != NULL);

      addend = 2 * (h->plt.offset & 1);
      h->plt.offset &= ~1;

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

      /* Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first two are reserved.  */
      got_offset = (plt_index + 3) * 4;

      /* Fill in the entry in the procedure linkage table.  */
      memcpy (splt->contents + h->plt.offset, elf_vax_plt_entry,
		  PLT_ENTRY_SIZE);

      /* The offset is relative to the first extension word.  */
      bfd_put_32 (output_bfd,
		  -(h->plt.offset + 8),
		  splt->contents + h->plt.offset + 4);

      bfd_put_32 (output_bfd, plt_index * sizeof (Elf32_External_Rela),
		  splt->contents + h->plt.offset + 8);

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset
		   + h->plt.offset) + addend,
		  sgot->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_VAX_JMP_SLOT);
      rela.r_addend = addend;
      loc = srela->contents + plt_index * sizeof (Elf32_External_Rela);
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
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      sgot = elf_hash_table (info)->sgot;
      srela = elf_hash_table (info)->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
		       + sgot->output_offset
		       + h->got.offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_VAX_GLOB_DAT);
      rela.r_addend = bfd_get_signed_32 (output_bfd,
					 sgot->contents + h->got.offset);

      loc = srela->contents;
      loc += srela->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
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

      s = bfd_get_linker_section (dynobj, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_VAX_COPY);
      rela.r_addend = 0;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (h == elf_hash_table (info)->hdynamic
      || h == elf_hash_table (info)->hgot)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
elf_vax_finish_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sgot;
  asection *sdyn;

  dynobj = elf_hash_table (info)->dynobj;

  sgot = elf_hash_table (info)->sgotplt;
  BFD_ASSERT (sgot != NULL);
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = elf_hash_table (info)->splt;
      BFD_ASSERT (splt != NULL && sdyn != NULL);

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

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->size > 0)
	{
	  memcpy (splt->contents, elf_vax_plt0_entry, PLT_ENTRY_SIZE);
	  bfd_put_32 (output_bfd,
			  (sgot->output_section->vma
			   + sgot->output_offset + 4
			   - (splt->output_section->vma + 6)),
			  splt->contents + 2);
	  bfd_put_32 (output_bfd,
			  (sgot->output_section->vma
			   + sgot->output_offset + 8
			   - (splt->output_section->vma + 12)),
			  splt->contents + 8);
	  elf_section_data (splt->output_section)->this_hdr.sh_entsize
	   = PLT_ENTRY_SIZE;
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
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 8);
    }

  if (elf_section_data (sgot->output_section) != NULL)
    elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;

  return true;
}

static enum elf_reloc_type_class
elf_vax_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			  const asection *rel_sec ATTRIBUTE_UNUSED,
			  const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_VAX_RELATIVE:
      return reloc_class_relative;
    case R_VAX_JMP_SLOT:
      return reloc_class_plt;
    case R_VAX_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

static bfd_vma
elf_vax_plt_sym_val (bfd_vma i, const asection *plt,
		     const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * PLT_ENTRY_SIZE;
}

#define TARGET_LITTLE_SYM		vax_elf32_vec
#define TARGET_LITTLE_NAME		"elf32-vax"
#define ELF_MACHINE_CODE		EM_VAX
#define ELF_MAXPAGESIZE			0x1000

#define elf_backend_create_dynamic_sections \
					_bfd_elf_create_dynamic_sections
#define bfd_elf32_bfd_link_hash_table_create \
					elf_vax_link_hash_table_create
#define bfd_elf32_bfd_final_link	bfd_elf_gc_common_final_link

#define elf_backend_check_relocs	elf_vax_check_relocs
#define elf_backend_adjust_dynamic_symbol \
					elf_vax_adjust_dynamic_symbol
#define elf_backend_always_size_sections \
					elf_vax_always_size_sections
#define elf_backend_size_dynamic_sections \
					elf_vax_size_dynamic_sections
#define elf_backend_init_index_section	_bfd_elf_init_1_index_section
#define elf_backend_relocate_section	elf_vax_relocate_section
#define elf_backend_finish_dynamic_symbol \
					elf_vax_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					elf_vax_finish_dynamic_sections
#define elf_backend_reloc_type_class	elf_vax_reloc_type_class
#define elf_backend_gc_mark_hook	elf_vax_gc_mark_hook
#define elf_backend_plt_sym_val		elf_vax_plt_sym_val
#define bfd_elf32_bfd_merge_private_bfd_data \
					elf32_vax_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags \
					elf32_vax_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data \
					elf32_vax_print_private_bfd_data

#define elf_backend_can_gc_sections	1
#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	16
#define elf_backend_rela_normal		1
#define elf_backend_dtrel_excludes_plt	1

#include "elf32-target.h"
