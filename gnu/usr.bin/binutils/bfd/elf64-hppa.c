/* Support for HPPA 64-bit ELF
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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
#include "elf/hppa.h"
#include "libhppa.h"
#include "elf64-hppa.h"
#include "libiberty.h"

#define ARCH_SIZE	       64

#define PLT_ENTRY_SIZE 0x10
#define DLT_ENTRY_SIZE 0x8
#define OPD_ENTRY_SIZE 0x20

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/pa20_64/dld.sl"

/* The stub is supposed to load the target address and target's DP
   value out of the PLT, then do an external branch to the target
   address.

   LDD PLTOFF(%r27),%r1
   BVE (%r1)
   LDD PLTOFF+8(%r27),%r27

   Note that we must use the LDD with a 14 bit displacement, not the one
   with a 5 bit displacement.  */
static char plt_stub[] = {0x53, 0x61, 0x00, 0x00, 0xe8, 0x20, 0xd0, 0x00,
			  0x53, 0x7b, 0x00, 0x00 };

struct elf64_hppa_link_hash_entry
{
  struct elf_link_hash_entry eh;

  /* Offsets for this symbol in various linker sections.  */
  bfd_vma dlt_offset;
  bfd_vma plt_offset;
  bfd_vma opd_offset;
  bfd_vma stub_offset;

  /* The index of the (possibly local) symbol in the input bfd and its
     associated BFD.  Needed so that we can have relocs against local
     symbols in shared libraries.  */
  long sym_indx;
  bfd *owner;

  /* Dynamic symbols may need to have two different values.  One for
     the dynamic symbol table, one for the normal symbol table.

     In such cases we store the symbol's real value and section
     index here so we can restore the real value before we write
     the normal symbol table.  */
  bfd_vma st_value;
  int st_shndx;

  /* Used to count non-got, non-plt relocations for delayed sizing
     of relocation sections.  */
  struct elf64_hppa_dyn_reloc_entry
  {
    /* Next relocation in the chain.  */
    struct elf64_hppa_dyn_reloc_entry *next;

    /* The type of the relocation.  */
    int type;

    /* The input section of the relocation.  */
    asection *sec;

    /* Number of relocs copied in this section.  */
    bfd_size_type count;

    /* The index of the section symbol for the input section of
       the relocation.  Only needed when building shared libraries.  */
    int sec_symndx;

    /* The offset within the input section of the relocation.  */
    bfd_vma offset;

    /* The addend for the relocation.  */
    bfd_vma addend;

  } *reloc_entries;

  /* Nonzero if this symbol needs an entry in one of the linker
     sections.  */
  unsigned want_dlt;
  unsigned want_plt;
  unsigned want_opd;
  unsigned want_stub;
};

struct elf64_hppa_link_hash_table
{
  struct elf_link_hash_table root;

  /* Shortcuts to get to the various linker defined sections.  */
  asection *dlt_sec;
  asection *dlt_rel_sec;
  asection *opd_sec;
  asection *opd_rel_sec;
  asection *other_rel_sec;

  /* Offset of __gp within .plt section.  When the PLT gets large we want
     to slide __gp into the PLT section so that we can continue to use
     single DP relative instructions to load values out of the PLT.  */
  bfd_vma gp_offset;

  /* Note this is not strictly correct.  We should create a stub section for
     each input section with calls.  The stub section should be placed before
     the section with the call.  */
  asection *stub_sec;

  bfd_vma text_segment_base;
  bfd_vma data_segment_base;

  /* We build tables to map from an input section back to its
     symbol index.  This is the BFD for which we currently have
     a map.  */
  bfd *section_syms_bfd;

  /* Array of symbol numbers for each input section attached to the
     current BFD.  */
  int *section_syms;
};

#define hppa_link_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == HPPA64_ELF_DATA)	\
   ? (struct elf64_hppa_link_hash_table *) (p)->hash : NULL)

#define hppa_elf_hash_entry(ent) \
  ((struct elf64_hppa_link_hash_entry *)(ent))

#define eh_name(eh) \
  (eh ? eh->root.root.string : "<undef>")

typedef struct bfd_hash_entry *(*new_hash_entry_func)
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);

static struct bfd_link_hash_table *elf64_hppa_hash_table_create
  (bfd *abfd);

/* This must follow the definitions of the various derived linker
   hash tables and shared functions.  */
#include "elf-hppa.h"

static bool elf64_hppa_object_p
  (bfd *);

static bool elf64_hppa_create_dynamic_sections
  (bfd *, struct bfd_link_info *);

static bool elf64_hppa_adjust_dynamic_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *);

static bool elf64_hppa_mark_milli_and_exported_functions
  (struct elf_link_hash_entry *, void *);

static bool elf64_hppa_size_dynamic_sections
  (bfd *, struct bfd_link_info *);

static int elf64_hppa_link_output_symbol_hook
  (struct bfd_link_info *, const char *, Elf_Internal_Sym *,
   asection *, struct elf_link_hash_entry *);

static bool elf64_hppa_finish_dynamic_symbol
  (bfd *, struct bfd_link_info *,
   struct elf_link_hash_entry *, Elf_Internal_Sym *);

static bool elf64_hppa_finish_dynamic_sections
  (bfd *, struct bfd_link_info *);

static bool elf64_hppa_check_relocs
  (bfd *, struct bfd_link_info *,
   asection *, const Elf_Internal_Rela *);

static bool elf64_hppa_dynamic_symbol_p
  (struct elf_link_hash_entry *, struct bfd_link_info *);

static bool elf64_hppa_mark_exported_functions
  (struct elf_link_hash_entry *, void *);

static bool elf64_hppa_finalize_opd
  (struct elf_link_hash_entry *, void *);

static bool elf64_hppa_finalize_dlt
  (struct elf_link_hash_entry *, void *);

static bool allocate_global_data_dlt
  (struct elf_link_hash_entry *, void *);

static bool allocate_global_data_plt
  (struct elf_link_hash_entry *, void *);

static bool allocate_global_data_stub
  (struct elf_link_hash_entry *, void *);

static bool allocate_global_data_opd
  (struct elf_link_hash_entry *, void *);

static bool get_reloc_section
  (bfd *, struct elf64_hppa_link_hash_table *, asection *);

static bool count_dyn_reloc
  (bfd *, struct elf64_hppa_link_hash_entry *,
   int, asection *, int, bfd_vma, bfd_vma);

static bool allocate_dynrel_entries
  (struct elf_link_hash_entry *, void *);

static bool elf64_hppa_finalize_dynreloc
  (struct elf_link_hash_entry *, void *);

static bool get_opd
  (bfd *, struct bfd_link_info *, struct elf64_hppa_link_hash_table *);

static bool get_plt
  (bfd *, struct bfd_link_info *, struct elf64_hppa_link_hash_table *);

static bool get_dlt
  (bfd *, struct bfd_link_info *, struct elf64_hppa_link_hash_table *);

static bool get_stub
  (bfd *, struct bfd_link_info *, struct elf64_hppa_link_hash_table *);

static int elf64_hppa_elf_get_symbol_type
  (Elf_Internal_Sym *, int);

/* Initialize an entry in the link hash table.  */

static struct bfd_hash_entry *
hppa64_link_hash_newfunc (struct bfd_hash_entry *entry,
			  struct bfd_hash_table *table,
			  const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf64_hppa_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf64_hppa_link_hash_entry *hh;

      /* Initialize our local data.  All zeros.  */
      hh = hppa_elf_hash_entry (entry);
      memset (&hh->dlt_offset, 0,
	      (sizeof (struct elf64_hppa_link_hash_entry)
	       - offsetof (struct elf64_hppa_link_hash_entry, dlt_offset)));
    }

  return entry;
}

/* Create the derived linker hash table.  The PA64 ELF port uses this
   derived hash table to keep information specific to the PA ElF
   linker (without using static variables).  */

static struct bfd_link_hash_table*
elf64_hppa_hash_table_create (bfd *abfd)
{
  struct elf64_hppa_link_hash_table *htab;
  size_t amt = sizeof (*htab);

  htab = bfd_zmalloc (amt);
  if (htab == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&htab->root, abfd,
				      hppa64_link_hash_newfunc,
				      sizeof (struct elf64_hppa_link_hash_entry),
				      HPPA64_ELF_DATA))
    {
      free (htab);
      return NULL;
    }

  htab->root.dt_pltgot_required = true;
  htab->text_segment_base = (bfd_vma) -1;
  htab->data_segment_base = (bfd_vma) -1;

  return &htab->root.root;
}

/* Return nonzero if ABFD represents a PA2.0 ELF64 file.

   Additionally we set the default architecture and machine.  */
static bool
elf64_hppa_object_p (bfd *abfd)
{
  Elf_Internal_Ehdr * i_ehdrp;
  unsigned int flags;

  i_ehdrp = elf_elfheader (abfd);
  if (strcmp (bfd_get_target (abfd), "elf64-hppa-linux") == 0)
    {
      /* GCC on hppa-linux produces binaries with OSABI=GNU,
	 but the kernel produces corefiles with OSABI=SysV.  */
      if (i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_GNU
	  && i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_NONE) /* aka SYSV */
	return false;
    }
  else
    {
      /* HPUX produces binaries with OSABI=HPUX,
	 but the kernel produces corefiles with OSABI=SysV.  */
      if (i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_HPUX
	  && i_ehdrp->e_ident[EI_OSABI] != ELFOSABI_NONE) /* aka SYSV */
	return false;
    }

  flags = i_ehdrp->e_flags;
  switch (flags & (EF_PARISC_ARCH | EF_PARISC_WIDE))
    {
    case EFA_PARISC_1_0:
      return bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 10);
    case EFA_PARISC_1_1:
      return bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 11);
    case EFA_PARISC_2_0:
      if (i_ehdrp->e_ident[EI_CLASS] == ELFCLASS64)
	return bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 25);
      else
	return bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 20);
    case EFA_PARISC_2_0 | EF_PARISC_WIDE:
      return bfd_default_set_arch_mach (abfd, bfd_arch_hppa, 25);
    }
  /* Don't be fussy.  */
  return true;
}

/* Given section type (hdr->sh_type), return a boolean indicating
   whether or not the section is an elf64-hppa specific section.  */
static bool
elf64_hppa_section_from_shdr (bfd *abfd,
			      Elf_Internal_Shdr *hdr,
			      const char *name,
			      int shindex)
{
  switch (hdr->sh_type)
    {
    case SHT_PARISC_EXT:
      if (strcmp (name, ".PARISC.archext") != 0)
	return false;
      break;
    case SHT_PARISC_UNWIND:
      if (strcmp (name, ".PARISC.unwind") != 0)
	return false;
      break;
    case SHT_PARISC_DOC:
    case SHT_PARISC_ANNOT:
    default:
      return false;
    }

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return false;

  return ((hdr->sh_flags & SHF_PARISC_SHORT) == 0
	  || bfd_set_section_flags (hdr->bfd_section,
				    hdr->bfd_section->flags | SEC_SMALL_DATA));
}

/* SEC is a section containing relocs for an input BFD when linking; return
   a suitable section for holding relocs in the output BFD for a link.  */

static bool
get_reloc_section (bfd *abfd,
		   struct elf64_hppa_link_hash_table *hppa_info,
		   asection *sec)
{
  const char *srel_name;
  asection *srel;
  bfd *dynobj;

  srel_name = (bfd_elf_string_from_elf_section
	       (abfd, elf_elfheader(abfd)->e_shstrndx,
		_bfd_elf_single_rel_hdr(sec)->sh_name));
  if (srel_name == NULL)
    return false;

  dynobj = hppa_info->root.dynobj;
  if (!dynobj)
    hppa_info->root.dynobj = dynobj = abfd;

  srel = bfd_get_linker_section (dynobj, srel_name);
  if (srel == NULL)
    {
      srel = bfd_make_section_anyway_with_flags (dynobj, srel_name,
						 (SEC_ALLOC
						  | SEC_LOAD
						  | SEC_HAS_CONTENTS
						  | SEC_IN_MEMORY
						  | SEC_LINKER_CREATED
						  | SEC_READONLY));
      if (srel == NULL
	  || !bfd_set_section_alignment (srel, 3))
	return false;
    }

  hppa_info->other_rel_sec = srel;
  return true;
}

/* Add a new entry to the list of dynamic relocations against DYN_H.

   We use this to keep a record of all the FPTR relocations against a
   particular symbol so that we can create FPTR relocations in the
   output file.  */

static bool
count_dyn_reloc (bfd *abfd,
		 struct elf64_hppa_link_hash_entry *hh,
		 int type,
		 asection *sec,
		 int sec_symndx,
		 bfd_vma offset,
		 bfd_vma addend)
{
  struct elf64_hppa_dyn_reloc_entry *rent;

  rent = (struct elf64_hppa_dyn_reloc_entry *)
  bfd_alloc (abfd, (bfd_size_type) sizeof (*rent));
  if (!rent)
    return false;

  rent->next = hh->reloc_entries;
  rent->type = type;
  rent->sec = sec;
  rent->sec_symndx = sec_symndx;
  rent->offset = offset;
  rent->addend = addend;
  hh->reloc_entries = rent;

  return true;
}

/* Return a pointer to the local DLT, PLT and OPD reference counts
   for ABFD.  Returns NULL if the storage allocation fails.  */

static bfd_signed_vma *
hppa64_elf_local_refcounts (bfd *abfd)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  bfd_signed_vma *local_refcounts;

  local_refcounts = elf_local_got_refcounts (abfd);
  if (local_refcounts == NULL)
    {
      bfd_size_type size;

      /* Allocate space for local DLT, PLT and OPD reference
	 counts.  Done this way to save polluting elf_obj_tdata
	 with another target specific pointer.  */
      size = symtab_hdr->sh_info;
      size *= 3 * sizeof (bfd_signed_vma);
      local_refcounts = bfd_zalloc (abfd, size);
      elf_local_got_refcounts (abfd) = local_refcounts;
    }
  return local_refcounts;
}

/* Scan the RELOCS and record the type of dynamic entries that each
   referenced symbol needs.  */

static bool
elf64_hppa_check_relocs (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  struct elf64_hppa_link_hash_table *hppa_info;
  const Elf_Internal_Rela *relend;
  Elf_Internal_Shdr *symtab_hdr;
  const Elf_Internal_Rela *rel;
  unsigned int sec_symndx;

  if (bfd_link_relocatable (info))
    return true;

  /* If this is the first dynamic object found in the link, create
     the special sections required for dynamic linking.  */
  if (! elf_hash_table (info)->dynamic_sections_created)
    {
      if (! _bfd_elf_link_create_dynamic_sections (abfd, info))
	return false;
    }

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* If necessary, build a new table holding section symbols indices
     for this BFD.  */

  if (bfd_link_pic (info) && hppa_info->section_syms_bfd != abfd)
    {
      unsigned long i;
      unsigned int highest_shndx;
      Elf_Internal_Sym *local_syms = NULL;
      Elf_Internal_Sym *isym, *isymend;
      bfd_size_type amt;

      /* We're done with the old cache of section index to section symbol
	 index information.  Free it.

	 ?!? Note we leak the last section_syms array.  Presumably we
	 could free it in one of the later routines in this file.  */
      free (hppa_info->section_syms);

      /* Read this BFD's local symbols.  */
      if (symtab_hdr->sh_info != 0)
	{
	  local_syms = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (local_syms == NULL)
	    local_syms = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					       symtab_hdr->sh_info, 0,
					       NULL, NULL, NULL);
	  if (local_syms == NULL)
	    return false;
	}

      /* Record the highest section index referenced by the local symbols.  */
      highest_shndx = 0;
      isymend = local_syms + symtab_hdr->sh_info;
      for (isym = local_syms; isym < isymend; isym++)
	{
	  if (isym->st_shndx > highest_shndx
	      && isym->st_shndx < SHN_LORESERVE)
	    highest_shndx = isym->st_shndx;
	}

      /* Allocate an array to hold the section index to section symbol index
	 mapping.  Bump by one since we start counting at zero.  */
      highest_shndx++;
      amt = highest_shndx;
      amt *= sizeof (int);
      hppa_info->section_syms = (int *) bfd_malloc (amt);

      /* Now walk the local symbols again.  If we find a section symbol,
	 record the index of the symbol into the section_syms array.  */
      for (i = 0, isym = local_syms; isym < isymend; i++, isym++)
	{
	  if (ELF_ST_TYPE (isym->st_info) == STT_SECTION)
	    hppa_info->section_syms[isym->st_shndx] = i;
	}

      /* We are finished with the local symbols.  */
      if (local_syms != NULL
	  && symtab_hdr->contents != (unsigned char *) local_syms)
	{
	  if (! info->keep_memory)
	    free (local_syms);
	  else
	    {
	      /* Cache the symbols for elf_link_input_bfd.  */
	      symtab_hdr->contents = (unsigned char *) local_syms;
	    }
	}

      /* Record which BFD we built the section_syms mapping for.  */
      hppa_info->section_syms_bfd = abfd;
    }

  /* Record the symbol index for this input section.  We may need it for
     relocations when building shared libraries.  When not building shared
     libraries this value is never really used, but assign it to zero to
     prevent out of bounds memory accesses in other routines.  */
  if (bfd_link_pic (info))
    {
      sec_symndx = _bfd_elf_section_from_bfd_section (abfd, sec);

      /* If we did not find a section symbol for this section, then
	 something went terribly wrong above.  */
      if (sec_symndx == SHN_BAD)
	return false;

      if (sec_symndx < SHN_LORESERVE)
	sec_symndx = hppa_info->section_syms[sec_symndx];
      else
	sec_symndx = 0;
    }
  else
    sec_symndx = 0;

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; ++rel)
    {
      enum
	{
	  NEED_DLT = 1,
	  NEED_PLT = 2,
	  NEED_STUB = 4,
	  NEED_OPD = 8,
	  NEED_DYNREL = 16,
	};

      unsigned long r_symndx = ELF64_R_SYM (rel->r_info);
      struct elf64_hppa_link_hash_entry *hh;
      int need_entry;
      bool maybe_dynamic;
      int dynrel_type = R_PARISC_NONE;
      static reloc_howto_type *howto;

      if (r_symndx >= symtab_hdr->sh_info)
	{
	  /* We're dealing with a global symbol -- find its hash entry
	     and mark it as being referenced.  */
	  long indx = r_symndx - symtab_hdr->sh_info;
	  hh = hppa_elf_hash_entry (elf_sym_hashes (abfd)[indx]);
	  while (hh->eh.root.type == bfd_link_hash_indirect
		 || hh->eh.root.type == bfd_link_hash_warning)
	    hh = hppa_elf_hash_entry (hh->eh.root.u.i.link);

	  /* PR15323, ref flags aren't set for references in the same
	     object.  */
	  hh->eh.ref_regular = 1;
	}
      else
	hh = NULL;

      /* We can only get preliminary data on whether a symbol is
	 locally or externally defined, as not all of the input files
	 have yet been processed.  Do something with what we know, as
	 this may help reduce memory usage and processing time later.  */
      maybe_dynamic = false;
      if (hh && ((bfd_link_pic (info)
		 && (!info->symbolic
		     || info->unresolved_syms_in_shared_libs == RM_IGNORE))
		|| !hh->eh.def_regular
		|| hh->eh.root.type == bfd_link_hash_defweak))
	maybe_dynamic = true;

      howto = elf_hppa_howto_table + ELF64_R_TYPE (rel->r_info);
      need_entry = 0;
      switch (howto->type)
	{
	/* These are simple indirect references to symbols through the
	   DLT.  We need to create a DLT entry for any symbols which
	   appears in a DLTIND relocation.  */
	case R_PARISC_DLTIND21L:
	case R_PARISC_DLTIND14R:
	case R_PARISC_DLTIND14F:
	case R_PARISC_DLTIND14WR:
	case R_PARISC_DLTIND14DR:
	  need_entry = NEED_DLT;
	  break;

	/* ?!?  These need a DLT entry.  But I have no idea what to do with
	   the "link time TP value.  */
	case R_PARISC_LTOFF_TP21L:
	case R_PARISC_LTOFF_TP14R:
	case R_PARISC_LTOFF_TP14F:
	case R_PARISC_LTOFF_TP64:
	case R_PARISC_LTOFF_TP14WR:
	case R_PARISC_LTOFF_TP14DR:
	case R_PARISC_LTOFF_TP16F:
	case R_PARISC_LTOFF_TP16WF:
	case R_PARISC_LTOFF_TP16DF:
	  need_entry = NEED_DLT;
	  break;

	/* These are function calls.  Depending on their precise target we
	   may need to make a stub for them.  The stub uses the PLT, so we
	   need to create PLT entries for these symbols too.  */
	case R_PARISC_PCREL12F:
	case R_PARISC_PCREL17F:
	case R_PARISC_PCREL22F:
	case R_PARISC_PCREL32:
	case R_PARISC_PCREL64:
	case R_PARISC_PCREL21L:
	case R_PARISC_PCREL17R:
	case R_PARISC_PCREL17C:
	case R_PARISC_PCREL14R:
	case R_PARISC_PCREL14F:
	case R_PARISC_PCREL22C:
	case R_PARISC_PCREL14WR:
	case R_PARISC_PCREL14DR:
	case R_PARISC_PCREL16F:
	case R_PARISC_PCREL16WF:
	case R_PARISC_PCREL16DF:
	  /* Function calls might need to go through the .plt, and
	     might need a long branch stub.  */
	  if (hh != NULL && hh->eh.type != STT_PARISC_MILLI)
	    need_entry = (NEED_PLT | NEED_STUB);
	  else
	    need_entry = 0;
	  break;

	case R_PARISC_PLTOFF21L:
	case R_PARISC_PLTOFF14R:
	case R_PARISC_PLTOFF14F:
	case R_PARISC_PLTOFF14WR:
	case R_PARISC_PLTOFF14DR:
	case R_PARISC_PLTOFF16F:
	case R_PARISC_PLTOFF16WF:
	case R_PARISC_PLTOFF16DF:
	  need_entry = (NEED_PLT);
	  break;

	case R_PARISC_DIR64:
	  if (bfd_link_pic (info) || maybe_dynamic)
	    need_entry = (NEED_DYNREL);
	  dynrel_type = R_PARISC_DIR64;
	  break;

	/* This is an indirect reference through the DLT to get the address
	   of a OPD descriptor.  Thus we need to make a DLT entry that points
	   to an OPD entry.  */
	case R_PARISC_LTOFF_FPTR21L:
	case R_PARISC_LTOFF_FPTR14R:
	case R_PARISC_LTOFF_FPTR14WR:
	case R_PARISC_LTOFF_FPTR14DR:
	case R_PARISC_LTOFF_FPTR32:
	case R_PARISC_LTOFF_FPTR64:
	case R_PARISC_LTOFF_FPTR16F:
	case R_PARISC_LTOFF_FPTR16WF:
	case R_PARISC_LTOFF_FPTR16DF:
	  if (bfd_link_pic (info) || maybe_dynamic)
	    need_entry = (NEED_DLT | NEED_OPD | NEED_PLT);
	  else
	    need_entry = (NEED_DLT | NEED_OPD | NEED_PLT);
	  dynrel_type = R_PARISC_FPTR64;
	  break;

	/* This is a simple OPD entry.  */
	case R_PARISC_FPTR64:
	  if (bfd_link_pic (info) || maybe_dynamic)
	    need_entry = (NEED_OPD | NEED_PLT | NEED_DYNREL);
	  else
	    need_entry = (NEED_OPD | NEED_PLT);
	  dynrel_type = R_PARISC_FPTR64;
	  break;

	/* Add more cases as needed.  */
	}

      if (!need_entry)
	continue;

      if (hh)
	{
	  /* Stash away enough information to be able to find this symbol
	     regardless of whether or not it is local or global.  */
	  hh->owner = abfd;
	  hh->sym_indx = r_symndx;
	}

      /* Create what's needed.  */
      if (need_entry & NEED_DLT)
	{
	  /* Allocate space for a DLT entry, as well as a dynamic
	     relocation for this entry.  */
	  if (! hppa_info->dlt_sec
	      && ! get_dlt (abfd, info, hppa_info))
	    goto err_out;

	  if (hh != NULL)
	    {
	      hh->want_dlt = 1;
	      hh->eh.got.refcount += 1;
	    }
	  else
	    {
	      bfd_signed_vma *local_dlt_refcounts;

	      /* This is a DLT entry for a local symbol.  */
	      local_dlt_refcounts = hppa64_elf_local_refcounts (abfd);
	      if (local_dlt_refcounts == NULL)
		return false;
	      local_dlt_refcounts[r_symndx] += 1;
	    }
	}

      if (need_entry & NEED_PLT)
	{
	  if (! hppa_info->root.splt
	      && ! get_plt (abfd, info, hppa_info))
	    goto err_out;

	  if (hh != NULL)
	    {
	      hh->want_plt = 1;
	      hh->eh.needs_plt = 1;
	      hh->eh.plt.refcount += 1;
	    }
	  else
	    {
	      bfd_signed_vma *local_dlt_refcounts;
	      bfd_signed_vma *local_plt_refcounts;

	      /* This is a PLT entry for a local symbol.  */
	      local_dlt_refcounts = hppa64_elf_local_refcounts (abfd);
	      if (local_dlt_refcounts == NULL)
		return false;
	      local_plt_refcounts = local_dlt_refcounts + symtab_hdr->sh_info;
	      local_plt_refcounts[r_symndx] += 1;
	    }
	}

      if (need_entry & NEED_STUB)
	{
	  if (! hppa_info->stub_sec
	      && ! get_stub (abfd, info, hppa_info))
	    goto err_out;
	  if (hh)
	    hh->want_stub = 1;
	}

      if (need_entry & NEED_OPD)
	{
	  if (! hppa_info->opd_sec
	      && ! get_opd (abfd, info, hppa_info))
	    goto err_out;

	  /* FPTRs are not allocated by the dynamic linker for PA64,
	     though it is possible that will change in the future.  */

	  if (hh != NULL)
	    hh->want_opd = 1;
	  else
	    {
	      bfd_signed_vma *local_dlt_refcounts;
	      bfd_signed_vma *local_opd_refcounts;

	      /* This is a OPD for a local symbol.  */
	      local_dlt_refcounts = hppa64_elf_local_refcounts (abfd);
	      if (local_dlt_refcounts == NULL)
		return false;
	      local_opd_refcounts = (local_dlt_refcounts
				     + 2 * symtab_hdr->sh_info);
	      local_opd_refcounts[r_symndx] += 1;
	    }
	}

      /* Add a new dynamic relocation to the chain of dynamic
	 relocations for this symbol.  */
      if ((need_entry & NEED_DYNREL) && (sec->flags & SEC_ALLOC))
	{
	  if (! hppa_info->other_rel_sec
	      && ! get_reloc_section (abfd, hppa_info, sec))
	    goto err_out;

	  /* Count dynamic relocations against global symbols.  */
	  if (hh != NULL
	      && !count_dyn_reloc (abfd, hh, dynrel_type, sec,
				   sec_symndx, rel->r_offset, rel->r_addend))
	    goto err_out;

	  /* If we are building a shared library and we just recorded
	     a dynamic R_PARISC_FPTR64 relocation, then make sure the
	     section symbol for this section ends up in the dynamic
	     symbol table.  */
	  if (bfd_link_pic (info) && dynrel_type == R_PARISC_FPTR64
	      && ! (bfd_elf_link_record_local_dynamic_symbol
		    (info, abfd, sec_symndx)))
	    return false;
	}
    }

  return true;

 err_out:
  return false;
}

struct elf64_hppa_allocate_data
{
  struct bfd_link_info *info;
  bfd_size_type ofs;
};

/* Should we do dynamic things to this symbol?  */

static bool
elf64_hppa_dynamic_symbol_p (struct elf_link_hash_entry *eh,
			     struct bfd_link_info *info)
{
  /* ??? What, if anything, needs to happen wrt STV_PROTECTED symbols
     and relocations that retrieve a function descriptor?  Assume the
     worst for now.  */
  if (_bfd_elf_dynamic_symbol_p (eh, info, 1))
    {
      /* ??? Why is this here and not elsewhere is_local_label_name.  */
      if (eh->root.root.string[0] == '$' && eh->root.root.string[1] == '$')
	return false;

      return true;
    }
  else
    return false;
}

/* Mark all functions exported by this file so that we can later allocate
   entries in .opd for them.  */

static bool
elf64_hppa_mark_exported_functions (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct bfd_link_info *info = (struct bfd_link_info *)data;
  struct elf64_hppa_link_hash_table *hppa_info;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  if (eh
      && (eh->root.type == bfd_link_hash_defined
	  || eh->root.type == bfd_link_hash_defweak)
      && eh->root.u.def.section->output_section != NULL
      && eh->type == STT_FUNC)
    {
      if (! hppa_info->opd_sec
	  && ! get_opd (hppa_info->root.dynobj, info, hppa_info))
	return false;

      hh->want_opd = 1;

      /* Put a flag here for output_symbol_hook.  */
      hh->st_shndx = -1;
      eh->needs_plt = 1;
    }

  return true;
}

/* Allocate space for a DLT entry.  */

static bool
allocate_global_data_dlt (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct elf64_hppa_allocate_data *x = (struct elf64_hppa_allocate_data *)data;

  if (hh->want_dlt)
    {
      if (bfd_link_pic (x->info))
	{
	  /* Possibly add the symbol to the local dynamic symbol
	     table since we might need to create a dynamic relocation
	     against it.  */
	  if (eh->dynindx == -1 && eh->type != STT_PARISC_MILLI)
	    {
	      bfd *owner = eh->root.u.def.section->owner;

	      if (! (bfd_elf_link_record_local_dynamic_symbol
		     (x->info, owner, hh->sym_indx)))
		return false;
	    }
	}

      hh->dlt_offset = x->ofs;
      x->ofs += DLT_ENTRY_SIZE;
    }
  return true;
}

/* Allocate space for a DLT.PLT entry.  */

static bool
allocate_global_data_plt (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct elf64_hppa_allocate_data *x = (struct elf64_hppa_allocate_data *) data;

  if (hh->want_plt
      && elf64_hppa_dynamic_symbol_p (eh, x->info)
      && !((eh->root.type == bfd_link_hash_defined
	    || eh->root.type == bfd_link_hash_defweak)
	   && eh->root.u.def.section->output_section != NULL))
    {
      hh->plt_offset = x->ofs;
      x->ofs += PLT_ENTRY_SIZE;
      if (hh->plt_offset < 0x2000)
	{
	  struct elf64_hppa_link_hash_table *hppa_info;

	  hppa_info = hppa_link_hash_table (x->info);
	  if (hppa_info == NULL)
	    return false;

	  hppa_info->gp_offset = hh->plt_offset;
	}
    }
  else
    hh->want_plt = 0;

  return true;
}

/* Allocate space for a STUB entry.  */

static bool
allocate_global_data_stub (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct elf64_hppa_allocate_data *x = (struct elf64_hppa_allocate_data *)data;

  if (hh->want_stub
      && elf64_hppa_dynamic_symbol_p (eh, x->info)
      && !((eh->root.type == bfd_link_hash_defined
	    || eh->root.type == bfd_link_hash_defweak)
	   && eh->root.u.def.section->output_section != NULL))
    {
      hh->stub_offset = x->ofs;
      x->ofs += sizeof (plt_stub);
    }
  else
    hh->want_stub = 0;
  return true;
}

/* Allocate space for a FPTR entry.  */

static bool
allocate_global_data_opd (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct elf64_hppa_allocate_data *x = (struct elf64_hppa_allocate_data *)data;

  if (hh && hh->want_opd)
    {
      /* We never need an opd entry for a symbol which is not
	 defined by this output file.  */
      if (hh && (hh->eh.root.type == bfd_link_hash_undefined
		 || hh->eh.root.type == bfd_link_hash_undefweak
		 || hh->eh.root.u.def.section->output_section == NULL))
	hh->want_opd = 0;

      /* If we are creating a shared library, took the address of a local
	 function or might export this function from this object file, then
	 we have to create an opd descriptor.  */
      else if (bfd_link_pic (x->info)
	       || hh == NULL
	       || (hh->eh.dynindx == -1 && hh->eh.type != STT_PARISC_MILLI)
	       || (hh->eh.root.type == bfd_link_hash_defined
		   || hh->eh.root.type == bfd_link_hash_defweak))
	{
	  /* If we are creating a shared library, then we will have to
	     create a runtime relocation for the symbol to properly
	     initialize the .opd entry.  Make sure the symbol gets
	     added to the dynamic symbol table.  */
	  if (bfd_link_pic (x->info)
	      && (hh == NULL || (hh->eh.dynindx == -1)))
	    {
	      bfd *owner;
	      /* PR 6511: Default to using the dynamic symbol table.  */
	      owner = (hh->owner ? hh->owner: eh->root.u.def.section->owner);

	      if (!bfd_elf_link_record_local_dynamic_symbol
		    (x->info, owner, hh->sym_indx))
		return false;
	    }

	  /* This may not be necessary or desirable anymore now that
	     we have some support for dealing with section symbols
	     in dynamic relocs.  But name munging does make the result
	     much easier to debug.  ie, the EPLT reloc will reference
	     a symbol like .foobar, instead of .text + offset.  */
	  if (bfd_link_pic (x->info) && eh)
	    {
	      char *new_name;
	      struct elf_link_hash_entry *nh;

	      new_name = concat (".", eh->root.root.string, NULL);

	      nh = elf_link_hash_lookup (elf_hash_table (x->info),
					 new_name, true, true, true);

	      free (new_name);
	      nh->root.type = eh->root.type;
	      nh->root.u.def.value = eh->root.u.def.value;
	      nh->root.u.def.section = eh->root.u.def.section;

	      if (! bfd_elf_link_record_dynamic_symbol (x->info, nh))
		return false;
	     }
	  hh->opd_offset = x->ofs;
	  x->ofs += OPD_ENTRY_SIZE;
	}

      /* Otherwise we do not need an opd entry.  */
      else
	hh->want_opd = 0;
    }
  return true;
}

/* HP requires the EI_OSABI field to be filled in.  The assignment to
   EI_ABIVERSION may not be strictly necessary.  */

static bool
elf64_hppa_init_file_header (bfd *abfd, struct bfd_link_info *info)
{
  Elf_Internal_Ehdr *i_ehdrp;

  if (!_bfd_elf_init_file_header (abfd, info))
    return false;

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_OSABI] = get_elf_backend_data (abfd)->elf_osabi;
  i_ehdrp->e_ident[EI_ABIVERSION] = 1;
  return true;
}

/* Create function descriptor section (.opd).  This section is called .opd
   because it contains "official procedure descriptors".  The "official"
   refers to the fact that these descriptors are used when taking the address
   of a procedure, thus ensuring a unique address for each procedure.  */

static bool
get_opd (bfd *abfd,
	 struct bfd_link_info *info ATTRIBUTE_UNUSED,
	 struct elf64_hppa_link_hash_table *hppa_info)
{
  asection *opd;
  bfd *dynobj;

  opd = hppa_info->opd_sec;
  if (!opd)
    {
      dynobj = hppa_info->root.dynobj;
      if (!dynobj)
	hppa_info->root.dynobj = dynobj = abfd;

      opd = bfd_make_section_anyway_with_flags (dynobj, ".opd",
						(SEC_ALLOC
						 | SEC_LOAD
						 | SEC_HAS_CONTENTS
						 | SEC_IN_MEMORY
						 | SEC_LINKER_CREATED));
      if (!opd
	  || !bfd_set_section_alignment (opd, 3))
	{
	  BFD_ASSERT (0);
	  return false;
	}

      hppa_info->opd_sec = opd;
    }

  return true;
}

/* Create the PLT section.  */

static bool
get_plt (bfd *abfd,
	 struct bfd_link_info *info ATTRIBUTE_UNUSED,
	 struct elf64_hppa_link_hash_table *hppa_info)
{
  asection *plt;
  bfd *dynobj;

  plt = hppa_info->root.splt;
  if (!plt)
    {
      dynobj = hppa_info->root.dynobj;
      if (!dynobj)
	hppa_info->root.dynobj = dynobj = abfd;

      plt = bfd_make_section_anyway_with_flags (dynobj, ".plt",
						(SEC_ALLOC
						 | SEC_LOAD
						 | SEC_HAS_CONTENTS
						 | SEC_IN_MEMORY
						 | SEC_LINKER_CREATED));
      if (!plt
	  || !bfd_set_section_alignment (plt, 3))
	{
	  BFD_ASSERT (0);
	  return false;
	}

      hppa_info->root.splt = plt;
    }

  return true;
}

/* Create the DLT section.  */

static bool
get_dlt (bfd *abfd,
	 struct bfd_link_info *info ATTRIBUTE_UNUSED,
	 struct elf64_hppa_link_hash_table *hppa_info)
{
  asection *dlt;
  bfd *dynobj;

  dlt = hppa_info->dlt_sec;
  if (!dlt)
    {
      dynobj = hppa_info->root.dynobj;
      if (!dynobj)
	hppa_info->root.dynobj = dynobj = abfd;

      dlt = bfd_make_section_anyway_with_flags (dynobj, ".dlt",
						(SEC_ALLOC
						 | SEC_LOAD
						 | SEC_HAS_CONTENTS
						 | SEC_IN_MEMORY
						 | SEC_LINKER_CREATED));
      if (!dlt
	  || !bfd_set_section_alignment (dlt, 3))
	{
	  BFD_ASSERT (0);
	  return false;
	}

      hppa_info->dlt_sec = dlt;
    }

  return true;
}

/* Create the stubs section.  */

static bool
get_stub (bfd *abfd,
	  struct bfd_link_info *info ATTRIBUTE_UNUSED,
	  struct elf64_hppa_link_hash_table *hppa_info)
{
  asection *stub;
  bfd *dynobj;

  stub = hppa_info->stub_sec;
  if (!stub)
    {
      dynobj = hppa_info->root.dynobj;
      if (!dynobj)
	hppa_info->root.dynobj = dynobj = abfd;

      stub = bfd_make_section_anyway_with_flags (dynobj, ".stub",
						 (SEC_ALLOC | SEC_LOAD
						  | SEC_HAS_CONTENTS
						  | SEC_IN_MEMORY
						  | SEC_READONLY
						  | SEC_LINKER_CREATED));
      if (!stub
	  || !bfd_set_section_alignment (stub, 3))
	{
	  BFD_ASSERT (0);
	  return false;
	}

      hppa_info->stub_sec = stub;
    }

  return true;
}

/* Create sections necessary for dynamic linking.  This is only a rough
   cut and will likely change as we learn more about the somewhat
   unusual dynamic linking scheme HP uses.

   .stub:
	Contains code to implement cross-space calls.  The first time one
	of the stubs is used it will call into the dynamic linker, later
	calls will go straight to the target.

	The only stub we support right now looks like

	ldd OFFSET(%dp),%r1
	bve %r0(%r1)
	ldd OFFSET+8(%dp),%dp

	Other stubs may be needed in the future.  We may want the remove
	the break/nop instruction.  It is only used right now to keep the
	offset of a .plt entry and a .stub entry in sync.

   .dlt:
	This is what most people call the .got.  HP used a different name.
	Losers.

   .rela.dlt:
	Relocations for the DLT.

   .plt:
	Function pointers as address,gp pairs.

   .rela.plt:
	Should contain dynamic IPLT (and EPLT?) relocations.

   .opd:
	FPTRS

   .rela.opd:
	EPLT relocations for symbols exported from shared libraries.  */

static bool
elf64_hppa_create_dynamic_sections (bfd *abfd,
				    struct bfd_link_info *info)
{
  asection *s;
  struct elf64_hppa_link_hash_table *hppa_info;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  if (! get_stub (abfd, info, hppa_info))
    return false;

  if (! get_dlt (abfd, info, hppa_info))
    return false;

  if (! get_plt (abfd, info, hppa_info))
    return false;

  if (! get_opd (abfd, info, hppa_info))
    return false;

  s = bfd_make_section_anyway_with_flags (abfd, ".rela.dlt",
					  (SEC_ALLOC | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  hppa_info->dlt_rel_sec = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".rela.plt",
					  (SEC_ALLOC | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  hppa_info->root.srelplt = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".rela.data",
					  (SEC_ALLOC | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  hppa_info->other_rel_sec = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".rela.opd",
					  (SEC_ALLOC | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY
					   | SEC_LINKER_CREATED));
  if (s == NULL
      || !bfd_set_section_alignment (s, 3))
    return false;
  hppa_info->opd_rel_sec = s;

  return true;
}

/* Allocate dynamic relocations for those symbols that turned out
   to be dynamic.  */

static bool
allocate_dynrel_entries (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct elf64_hppa_allocate_data *x = (struct elf64_hppa_allocate_data *)data;
  struct elf64_hppa_link_hash_table *hppa_info;
  struct elf64_hppa_dyn_reloc_entry *rent;
  bool dynamic_symbol, shared;

  hppa_info = hppa_link_hash_table (x->info);
  if (hppa_info == NULL)
    return false;

  dynamic_symbol = elf64_hppa_dynamic_symbol_p (eh, x->info);
  shared = bfd_link_pic (x->info);

  /* We may need to allocate relocations for a non-dynamic symbol
     when creating a shared library.  */
  if (!dynamic_symbol && !shared)
    return true;

  /* Take care of the normal data relocations.  */

  for (rent = hh->reloc_entries; rent; rent = rent->next)
    {
      /* Allocate one iff we are building a shared library, the relocation
	 isn't a R_PARISC_FPTR64, or we don't want an opd entry.  */
      if (!shared && rent->type == R_PARISC_FPTR64 && hh->want_opd)
	continue;

      hppa_info->other_rel_sec->size += sizeof (Elf64_External_Rela);

      /* Make sure this symbol gets into the dynamic symbol table if it is
	 not already recorded.  ?!? This should not be in the loop since
	 the symbol need only be added once.  */
      if (eh->dynindx == -1 && eh->type != STT_PARISC_MILLI)
	if (!bfd_elf_link_record_local_dynamic_symbol
	    (x->info, rent->sec->owner, hh->sym_indx))
	  return false;
    }

  /* Take care of the GOT and PLT relocations.  */

  if ((dynamic_symbol || shared) && hh->want_dlt)
    hppa_info->dlt_rel_sec->size += sizeof (Elf64_External_Rela);

  /* If we are building a shared library, then every symbol that has an
     opd entry will need an EPLT relocation to relocate the symbol's address
     and __gp value based on the runtime load address.  */
  if (shared && hh->want_opd)
    hppa_info->opd_rel_sec->size += sizeof (Elf64_External_Rela);

  if (hh->want_plt && dynamic_symbol)
    {
      bfd_size_type t = 0;

      /* Dynamic symbols get one IPLT relocation.  Local symbols in
	 shared libraries get two REL relocations.  Local symbols in
	 main applications get nothing.  */
      if (dynamic_symbol)
	t = sizeof (Elf64_External_Rela);
      else if (shared)
	t = 2 * sizeof (Elf64_External_Rela);

      hppa_info->root.srelplt->size += t;
    }

  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  */

static bool
elf64_hppa_adjust_dynamic_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  struct elf_link_hash_entry *eh)
{
  /* ??? Undefined symbols with PLT entries should be re-defined
     to be the PLT entry.  */

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (eh->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (eh);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      eh->root.u.def.section = def->root.u.def.section;
      eh->root.u.def.value = def->root.u.def.value;
      return true;
    }

  /* If this is a reference to a symbol defined by a dynamic object which
     is not a function, we might allocate the symbol in our .dynbss section
     and allocate a COPY dynamic relocation.

     But PA64 code is canonically PIC, so as a rule we can avoid this sort
     of hackery.  */

  return true;
}

/* This function is called via elf_link_hash_traverse to mark millicode
   symbols with a dynindx of -1 and to remove the string table reference
   from the dynamic symbol table.  If the symbol is not a millicode symbol,
   elf64_hppa_mark_exported_functions is called.  */

static bool
elf64_hppa_mark_milli_and_exported_functions (struct elf_link_hash_entry *eh,
					      void *data)
{
  struct bfd_link_info *info = (struct bfd_link_info *) data;

  if (eh->type == STT_PARISC_MILLI)
    {
      if (eh->dynindx != -1)
	{
	  eh->dynindx = -1;
	  _bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
				  eh->dynstr_index);
	}
      return true;
    }

  return elf64_hppa_mark_exported_functions (eh, data);
}

/* Set the final sizes of the dynamic sections and allocate memory for
   the contents of our special sections.  */

static bool
elf64_hppa_size_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  struct elf64_hppa_link_hash_table *hppa_info;
  struct elf64_hppa_allocate_data data;
  bfd *dynobj;
  bfd *ibfd;
  asection *sec;
  bool relocs;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  dynobj = hppa_info->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  /* Mark each function this program exports so that we will allocate
     space in the .opd section for each function's FPTR.  If we are
     creating dynamic sections, change the dynamic index of millicode
     symbols to -1 and remove them from the string table for .dynstr.

     We have to traverse the main linker hash table since we have to
     find functions which may not have been mentioned in any relocs.  */
  elf_link_hash_traverse (&hppa_info->root,
			  (hppa_info->root.dynamic_sections_created
			   ? elf64_hppa_mark_milli_and_exported_functions
			   : elf64_hppa_mark_exported_functions),
			  info);

  if (hppa_info->root.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  sec = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (sec != NULL);
	  sec->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  sec->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }
  else
    {
      /* We may have created entries in the .rela.got section.
	 However, if we are not creating the dynamic sections, we will
	 not actually use these entries.  Reset the size of .rela.dlt,
	 which will cause it to get stripped from the output file
	 below.  */
      sec = hppa_info->dlt_rel_sec;
      if (sec != NULL)
	sec->size = 0;
    }

  /* Set up DLT, PLT and OPD offsets for local syms, and space for local
     dynamic relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_dlt;
      bfd_signed_vma *end_local_dlt;
      bfd_signed_vma *local_plt;
      bfd_signed_vma *end_local_plt;
      bfd_signed_vma *local_opd;
      bfd_signed_vma *end_local_opd;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  struct elf64_hppa_dyn_reloc_entry *hdh_p;

	  for (hdh_p = ((struct elf64_hppa_dyn_reloc_entry *)
		    elf_section_data (sec)->local_dynrel);
	       hdh_p != NULL;
	       hdh_p = hdh_p->next)
	    {
	      if (!bfd_is_abs_section (hdh_p->sec)
		  && bfd_is_abs_section (hdh_p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (hdh_p->count != 0)
		{
		  srel = elf_section_data (hdh_p->sec)->sreloc;
		  srel->size += hdh_p->count * sizeof (Elf64_External_Rela);
		  if ((hdh_p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_dlt = elf_local_got_refcounts (ibfd);
      if (!local_dlt)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_dlt = local_dlt + locsymcount;
      sec = hppa_info->dlt_sec;
      srel = hppa_info->dlt_rel_sec;
      for (; local_dlt < end_local_dlt; ++local_dlt)
	{
	  if (*local_dlt > 0)
	    {
	      *local_dlt = sec->size;
	      sec->size += DLT_ENTRY_SIZE;
	      if (bfd_link_pic (info))
		{
		  srel->size += sizeof (Elf64_External_Rela);
		}
	    }
	  else
	    *local_dlt = (bfd_vma) -1;
	}

      local_plt = end_local_dlt;
      end_local_plt = local_plt + locsymcount;
      if (! hppa_info->root.dynamic_sections_created)
	{
	  /* Won't be used, but be safe.  */
	  for (; local_plt < end_local_plt; ++local_plt)
	    *local_plt = (bfd_vma) -1;
	}
      else
	{
	  sec = hppa_info->root.splt;
	  srel = hppa_info->root.srelplt;
	  for (; local_plt < end_local_plt; ++local_plt)
	    {
	      if (*local_plt > 0)
		{
		  *local_plt = sec->size;
		  sec->size += PLT_ENTRY_SIZE;
		  if (bfd_link_pic (info))
		    srel->size += sizeof (Elf64_External_Rela);
		}
	      else
		*local_plt = (bfd_vma) -1;
	    }
	}

      local_opd = end_local_plt;
      end_local_opd = local_opd + locsymcount;
      if (! hppa_info->root.dynamic_sections_created)
	{
	  /* Won't be used, but be safe.  */
	  for (; local_opd < end_local_opd; ++local_opd)
	    *local_opd = (bfd_vma) -1;
	}
      else
	{
	  sec = hppa_info->opd_sec;
	  srel = hppa_info->opd_rel_sec;
	  for (; local_opd < end_local_opd; ++local_opd)
	    {
	      if (*local_opd > 0)
		{
		  *local_opd = sec->size;
		  sec->size += OPD_ENTRY_SIZE;
		  if (bfd_link_pic (info))
		    srel->size += sizeof (Elf64_External_Rela);
		}
	      else
		*local_opd = (bfd_vma) -1;
	    }
	}
    }

  /* Allocate the GOT entries.  */

  data.info = info;
  if (hppa_info->dlt_sec)
    {
      data.ofs = hppa_info->dlt_sec->size;
      elf_link_hash_traverse (&hppa_info->root,
			      allocate_global_data_dlt, &data);
      hppa_info->dlt_sec->size = data.ofs;
    }

  if (hppa_info->root.splt)
    {
      data.ofs = hppa_info->root.splt->size;
      elf_link_hash_traverse (&hppa_info->root,
			      allocate_global_data_plt, &data);
      hppa_info->root.splt->size = data.ofs;
    }

  if (hppa_info->stub_sec)
    {
      data.ofs = 0x0;
      elf_link_hash_traverse (&hppa_info->root,
			      allocate_global_data_stub, &data);
      hppa_info->stub_sec->size = data.ofs;
    }

  /* Allocate space for entries in the .opd section.  */
  if (hppa_info->opd_sec)
    {
      data.ofs = hppa_info->opd_sec->size;
      elf_link_hash_traverse (&hppa_info->root,
			      allocate_global_data_opd, &data);
      hppa_info->opd_sec->size = data.ofs;
    }

  /* Now allocate space for dynamic relocations, if necessary.  */
  if (hppa_info->root.dynamic_sections_created)
    elf_link_hash_traverse (&hppa_info->root,
			    allocate_dynrel_entries, &data);

  /* The sizes of all the sections are set.  Allocate memory for them.  */
  relocs = false;
  for (sec = dynobj->sections; sec != NULL; sec = sec->next)
    {
      const char *name;

      if ((sec->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (sec);

      if (strcmp (name, ".plt") == 0)
	{
	  /* Remember whether there is a PLT.  */
	  ;
	}
      else if (strcmp (name, ".opd") == 0
	       || startswith (name, ".dlt")
	       || strcmp (name, ".stub") == 0
	       || strcmp (name, ".got") == 0)
	{
	  /* Strip this section if we don't need it; see the comment below.  */
	}
      else if (startswith (name, ".rela"))
	{
	  if (sec->size != 0)
	    {
	      /* Remember whether there are any reloc sections other
		 than .rela.plt.  */
	      if (strcmp (name, ".rela.plt") != 0)
		relocs = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      sec->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (sec->size == 0)
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
	  sec->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((sec->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents if it has not
	 been allocated already.  We use bfd_zalloc here in case
	 unused entries are not reclaimed before the section's
	 contents are written out.  This should not happen, but this
	 way if it does, we get a R_PARISC_NONE reloc instead of
	 garbage.  */
      if (sec->contents == NULL)
	{
	  sec->contents = (bfd_byte *) bfd_zalloc (dynobj, sec->size);
	  if (sec->contents == NULL)
	    return false;
	}
    }

  if (hppa_info->root.dynamic_sections_created)
    {
      /* Always create a DT_PLTGOT.  It actually has nothing to do with
	 the PLT, it is how we communicate the __gp value of a load
	 module to the dynamic linker.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (!add_dynamic_entry (DT_HP_DLD_FLAGS, 0))
	return false;

      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in elf64_hppa_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
      if (! bfd_link_pic (info))
	{
	  if (!add_dynamic_entry (DT_HP_DLD_HOOK, 0)
	      || !add_dynamic_entry (DT_HP_LOAD_MAP, 0))
	    return false;
	}

      /* Force DT_FLAGS to always be set.
	 Required by HPUX 11.00 patch PHSS_26559.  */
      if (!add_dynamic_entry (DT_FLAGS, (info)->flags))
	return false;
    }
#undef add_dynamic_entry

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Called after we have output the symbol into the dynamic symbol
   table, but before we output the symbol into the normal symbol
   table.

   For some symbols we had to change their address when outputting
   the dynamic symbol table.  We undo that change here so that
   the symbols have their expected value in the normal symbol
   table.  Ick.  */

static int
elf64_hppa_link_output_symbol_hook (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				    const char *name,
				    Elf_Internal_Sym *sym,
				    asection *input_sec ATTRIBUTE_UNUSED,
				    struct elf_link_hash_entry *eh)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);

  /* We may be called with the file symbol or section symbols.
     They never need munging, so it is safe to ignore them.  */
  if (!name || !eh)
    return 1;

  /* Function symbols for which we created .opd entries *may* have been
     munged by finish_dynamic_symbol and have to be un-munged here.

     Note that finish_dynamic_symbol sometimes turns dynamic symbols
     into non-dynamic ones, so we initialize st_shndx to -1 in
     mark_exported_functions and check to see if it was overwritten
     here instead of just checking eh->dynindx.  */
  if (hh->want_opd && hh->st_shndx != -1)
    {
      /* Restore the saved value and section index.  */
      sym->st_value = hh->st_value;
      sym->st_shndx = hh->st_shndx;
    }

  return 1;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf64_hppa_finish_dynamic_symbol (bfd *output_bfd,
				  struct bfd_link_info *info,
				  struct elf_link_hash_entry *eh,
				  Elf_Internal_Sym *sym)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  asection *stub, *splt, *sopd, *spltrel;
  struct elf64_hppa_link_hash_table *hppa_info;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  stub = hppa_info->stub_sec;
  splt = hppa_info->root.splt;
  sopd = hppa_info->opd_sec;
  spltrel = hppa_info->root.srelplt;

  /* Incredible.  It is actually necessary to NOT use the symbol's real
     value when building the dynamic symbol table for a shared library.
     At least for symbols that refer to functions.

     We will store a new value and section index into the symbol long
     enough to output it into the dynamic symbol table, then we restore
     the original values (in elf64_hppa_link_output_symbol_hook).  */
  if (hh->want_opd)
    {
      BFD_ASSERT (sopd != NULL);

      /* Save away the original value and section index so that we
	 can restore them later.  */
      hh->st_value = sym->st_value;
      hh->st_shndx = sym->st_shndx;

      /* For the dynamic symbol table entry, we want the value to be
	 address of this symbol's entry within the .opd section.  */
      sym->st_value = (hh->opd_offset
		       + sopd->output_offset
		       + sopd->output_section->vma);
      sym->st_shndx = _bfd_elf_section_from_bfd_section (output_bfd,
							 sopd->output_section);
    }

  /* Initialize a .plt entry if requested.  */
  if (hh->want_plt
      && elf64_hppa_dynamic_symbol_p (eh, info))
    {
      bfd_vma value;
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      BFD_ASSERT (splt != NULL && spltrel != NULL);

      /* We do not actually care about the value in the PLT entry
	 if we are creating a shared library and the symbol is
	 still undefined, we create a dynamic relocation to fill
	 in the correct value.  */
      if (bfd_link_pic (info) && eh->root.type == bfd_link_hash_undefined)
	value = 0;
      else
	value = (eh->root.u.def.value + eh->root.u.def.section->vma);

      /* Fill in the entry in the procedure linkage table.

	 The format of a plt entry is
	 <funcaddr> <__gp>.

	 plt_offset is the offset within the PLT section at which to
	 install the PLT entry.

	 We are modifying the in-memory PLT contents here, so we do not add
	 in the output_offset of the PLT section.  */

      bfd_put_64 (splt->owner, value, splt->contents + hh->plt_offset);
      value = _bfd_get_gp_value (info->output_bfd);
      bfd_put_64 (splt->owner, value, splt->contents + hh->plt_offset + 0x8);

      /* Create a dynamic IPLT relocation for this entry.

	 We are creating a relocation in the output file's PLT section,
	 which is included within the DLT secton.  So we do need to include
	 the PLT's output_offset in the computation of the relocation's
	 address.  */
      rel.r_offset = (hh->plt_offset + splt->output_offset
		      + splt->output_section->vma);
      rel.r_info = ELF64_R_INFO (hh->eh.dynindx, R_PARISC_IPLT);
      rel.r_addend = 0;

      loc = spltrel->contents;
      loc += spltrel->reloc_count++ * sizeof (Elf64_External_Rela);
      bfd_elf64_swap_reloca_out (info->output_bfd, &rel, loc);
    }

  /* Initialize an external call stub entry if requested.  */
  if (hh->want_stub
      && elf64_hppa_dynamic_symbol_p (eh, info))
    {
      bfd_vma value;
      int insn;
      unsigned int max_offset;

      BFD_ASSERT (stub != NULL);

      /* Install the generic stub template.

	 We are modifying the contents of the stub section, so we do not
	 need to include the stub section's output_offset here.  */
      memcpy (stub->contents + hh->stub_offset, plt_stub, sizeof (plt_stub));

      /* Fix up the first ldd instruction.

	 We are modifying the contents of the STUB section in memory,
	 so we do not need to include its output offset in this computation.

	 Note the plt_offset value is the value of the PLT entry relative to
	 the start of the PLT section.  These instructions will reference
	 data relative to the value of __gp, which may not necessarily have
	 the same address as the start of the PLT section.

	 gp_offset contains the offset of __gp within the PLT section.  */
      value = hh->plt_offset - hppa_info->gp_offset;

      insn = bfd_get_32 (stub->owner, stub->contents + hh->stub_offset);
      if (output_bfd->arch_info->mach >= 25)
	{
	  /* Wide mode allows 16 bit offsets.  */
	  max_offset = 32768;
	  insn &= ~ 0xfff1;
	  insn |= re_assemble_16 ((int) value);
	}
      else
	{
	  max_offset = 8192;
	  insn &= ~ 0x3ff1;
	  insn |= re_assemble_14 ((int) value);
	}

      if ((value & 7) || value + max_offset >= 2*max_offset - 8)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("stub entry for %s cannot load .plt, dp offset = %" PRId64),
	     hh->eh.root.root.string, (int64_t) value);
	  return false;
	}

      bfd_put_32 (stub->owner, (bfd_vma) insn,
		  stub->contents + hh->stub_offset);

      /* Fix up the second ldd instruction.  */
      value += 8;
      insn = bfd_get_32 (stub->owner, stub->contents + hh->stub_offset + 8);
      if (output_bfd->arch_info->mach >= 25)
	{
	  insn &= ~ 0xfff1;
	  insn |= re_assemble_16 ((int) value);
	}
      else
	{
	  insn &= ~ 0x3ff1;
	  insn |= re_assemble_14 ((int) value);
	}
      bfd_put_32 (stub->owner, (bfd_vma) insn,
		  stub->contents + hh->stub_offset + 8);
    }

  return true;
}

/* The .opd section contains FPTRs for each function this file
   exports.  Initialize the FPTR entries.  */

static bool
elf64_hppa_finalize_opd (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct bfd_link_info *info = (struct bfd_link_info *)data;
  struct elf64_hppa_link_hash_table *hppa_info;
  asection *sopd;
  asection *sopdrel;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  sopd = hppa_info->opd_sec;
  sopdrel = hppa_info->opd_rel_sec;

  if (hh->want_opd)
    {
      bfd_vma value;

      /* The first two words of an .opd entry are zero.

	 We are modifying the contents of the OPD section in memory, so we
	 do not need to include its output offset in this computation.  */
      memset (sopd->contents + hh->opd_offset, 0, 16);

      value = (eh->root.u.def.value
	       + eh->root.u.def.section->output_section->vma
	       + eh->root.u.def.section->output_offset);

      /* The next word is the address of the function.  */
      bfd_put_64 (sopd->owner, value, sopd->contents + hh->opd_offset + 16);

      /* The last word is our local __gp value.  */
      value = _bfd_get_gp_value (info->output_bfd);
      bfd_put_64 (sopd->owner, value, sopd->contents + hh->opd_offset + 24);
    }

  /* If we are generating a shared library, we must generate EPLT relocations
     for each entry in the .opd, even for static functions (they may have
     had their address taken).  */
  if (bfd_link_pic (info) && hh->want_opd)
    {
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      int dynindx;

      /* We may need to do a relocation against a local symbol, in
	 which case we have to look up it's dynamic symbol index off
	 the local symbol hash table.  */
      if (eh->dynindx != -1)
	dynindx = eh->dynindx;
      else
	dynindx
	  = _bfd_elf_link_lookup_local_dynindx (info, hh->owner,
						hh->sym_indx);

      /* The offset of this relocation is the absolute address of the
	 .opd entry for this symbol.  */
      rel.r_offset = (hh->opd_offset + sopd->output_offset
		      + sopd->output_section->vma);

      /* If H is non-null, then we have an external symbol.

	 It is imperative that we use a different dynamic symbol for the
	 EPLT relocation if the symbol has global scope.

	 In the dynamic symbol table, the function symbol will have a value
	 which is address of the function's .opd entry.

	 Thus, we can not use that dynamic symbol for the EPLT relocation
	 (if we did, the data in the .opd would reference itself rather
	 than the actual address of the function).  Instead we have to use
	 a new dynamic symbol which has the same value as the original global
	 function symbol.

	 We prefix the original symbol with a "." and use the new symbol in
	 the EPLT relocation.  This new symbol has already been recorded in
	 the symbol table, we just have to look it up and use it.

	 We do not have such problems with static functions because we do
	 not make their addresses in the dynamic symbol table point to
	 the .opd entry.  Ultimately this should be safe since a static
	 function can not be directly referenced outside of its shared
	 library.

	 We do have to play similar games for FPTR relocations in shared
	 libraries, including those for static symbols.  See the FPTR
	 handling in elf64_hppa_finalize_dynreloc.  */
      if (eh)
	{
	  char *new_name;
	  struct elf_link_hash_entry *nh;

	  new_name = concat (".", eh->root.root.string, NULL);

	  nh = elf_link_hash_lookup (elf_hash_table (info),
				     new_name, true, true, false);

	  /* All we really want from the new symbol is its dynamic
	     symbol index.  */
	  if (nh)
	    dynindx = nh->dynindx;
	  free (new_name);
	}

      rel.r_addend = 0;
      rel.r_info = ELF64_R_INFO (dynindx, R_PARISC_EPLT);

      loc = sopdrel->contents;
      loc += sopdrel->reloc_count++ * sizeof (Elf64_External_Rela);
      bfd_elf64_swap_reloca_out (info->output_bfd, &rel, loc);
    }
  return true;
}

/* The .dlt section contains addresses for items referenced through the
   dlt.  Note that we can have a DLTIND relocation for a local symbol, thus
   we can not depend on finish_dynamic_symbol to initialize the .dlt.  */

static bool
elf64_hppa_finalize_dlt (struct elf_link_hash_entry *eh, void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct bfd_link_info *info = (struct bfd_link_info *)data;
  struct elf64_hppa_link_hash_table *hppa_info;
  asection *sdlt, *sdltrel;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  sdlt = hppa_info->dlt_sec;
  sdltrel = hppa_info->dlt_rel_sec;

  /* H/DYN_H may refer to a local variable and we know it's
     address, so there is no need to create a relocation.  Just install
     the proper value into the DLT, note this shortcut can not be
     skipped when building a shared library.  */
  if (! bfd_link_pic (info) && hh && hh->want_dlt)
    {
      bfd_vma value;

      /* If we had an LTOFF_FPTR style relocation we want the DLT entry
	 to point to the FPTR entry in the .opd section.

	 We include the OPD's output offset in this computation as
	 we are referring to an absolute address in the resulting
	 object file.  */
      if (hh->want_opd)
	{
	  value = (hh->opd_offset
		   + hppa_info->opd_sec->output_offset
		   + hppa_info->opd_sec->output_section->vma);
	}
      else if ((eh->root.type == bfd_link_hash_defined
		|| eh->root.type == bfd_link_hash_defweak)
	       && eh->root.u.def.section)
	{
	  value = eh->root.u.def.value + eh->root.u.def.section->output_offset;
	  if (eh->root.u.def.section->output_section)
	    value += eh->root.u.def.section->output_section->vma;
	  else
	    value += eh->root.u.def.section->vma;
	}
      else
	/* We have an undefined function reference.  */
	value = 0;

      /* We do not need to include the output offset of the DLT section
	 here because we are modifying the in-memory contents.  */
      bfd_put_64 (sdlt->owner, value, sdlt->contents + hh->dlt_offset);
    }

  /* Create a relocation for the DLT entry associated with this symbol.
     When building a shared library the symbol does not have to be dynamic.  */
  if (hh->want_dlt
      && (elf64_hppa_dynamic_symbol_p (eh, info) || bfd_link_pic (info)))
    {
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      int dynindx;

      /* We may need to do a relocation against a local symbol, in
	 which case we have to look up it's dynamic symbol index off
	 the local symbol hash table.  */
      if (eh && eh->dynindx != -1)
	dynindx = eh->dynindx;
      else
	dynindx
	  = _bfd_elf_link_lookup_local_dynindx (info, hh->owner,
						hh->sym_indx);

      /* Create a dynamic relocation for this entry.  Do include the output
	 offset of the DLT entry since we need an absolute address in the
	 resulting object file.  */
      rel.r_offset = (hh->dlt_offset + sdlt->output_offset
		      + sdlt->output_section->vma);
      if (eh && eh->type == STT_FUNC)
	  rel.r_info = ELF64_R_INFO (dynindx, R_PARISC_FPTR64);
      else
	  rel.r_info = ELF64_R_INFO (dynindx, R_PARISC_DIR64);
      rel.r_addend = 0;

      loc = sdltrel->contents;
      loc += sdltrel->reloc_count++ * sizeof (Elf64_External_Rela);
      bfd_elf64_swap_reloca_out (info->output_bfd, &rel, loc);
    }
  return true;
}

/* Finalize the dynamic relocations.  Specifically the FPTR relocations
   for dynamic functions used to initialize static data.  */

static bool
elf64_hppa_finalize_dynreloc (struct elf_link_hash_entry *eh,
			      void *data)
{
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  struct bfd_link_info *info = (struct bfd_link_info *)data;
  struct elf64_hppa_link_hash_table *hppa_info;
  int dynamic_symbol;

  dynamic_symbol = elf64_hppa_dynamic_symbol_p (eh, info);

  if (!dynamic_symbol && !bfd_link_pic (info))
    return true;

  if (hh->reloc_entries)
    {
      struct elf64_hppa_dyn_reloc_entry *rent;
      int dynindx;

      hppa_info = hppa_link_hash_table (info);
      if (hppa_info == NULL)
	return false;

      /* We may need to do a relocation against a local symbol, in
	 which case we have to look up it's dynamic symbol index off
	 the local symbol hash table.  */
      if (eh->dynindx != -1)
	dynindx = eh->dynindx;
      else
	dynindx
	  = _bfd_elf_link_lookup_local_dynindx (info, hh->owner,
						hh->sym_indx);

      for (rent = hh->reloc_entries; rent; rent = rent->next)
	{
	  Elf_Internal_Rela rel;
	  bfd_byte *loc;

	  /* Allocate one iff we are building a shared library, the relocation
	     isn't a R_PARISC_FPTR64, or we don't want an opd entry.  */
	  if (!bfd_link_pic (info)
	      && rent->type == R_PARISC_FPTR64 && hh->want_opd)
	    continue;

	  /* Create a dynamic relocation for this entry.

	     We need the output offset for the reloc's section because
	     we are creating an absolute address in the resulting object
	     file.  */
	  rel.r_offset = (rent->offset + rent->sec->output_offset
			  + rent->sec->output_section->vma);

	  /* An FPTR64 relocation implies that we took the address of
	     a function and that the function has an entry in the .opd
	     section.  We want the FPTR64 relocation to reference the
	     entry in .opd.

	     We could munge the symbol value in the dynamic symbol table
	     (in fact we already do for functions with global scope) to point
	     to the .opd entry.  Then we could use that dynamic symbol in
	     this relocation.

	     Or we could do something sensible, not munge the symbol's
	     address and instead just use a different symbol to reference
	     the .opd entry.  At least that seems sensible until you
	     realize there's no local dynamic symbols we can use for that
	     purpose.  Thus the hair in the check_relocs routine.

	     We use a section symbol recorded by check_relocs as the
	     base symbol for the relocation.  The addend is the difference
	     between the section symbol and the address of the .opd entry.  */
	  if (bfd_link_pic (info)
	      && rent->type == R_PARISC_FPTR64 && hh->want_opd)
	    {
	      bfd_vma value, value2;

	      /* First compute the address of the opd entry for this symbol.  */
	      value = (hh->opd_offset
		       + hppa_info->opd_sec->output_section->vma
		       + hppa_info->opd_sec->output_offset);

	      /* Compute the value of the start of the section with
		 the relocation.  */
	      value2 = (rent->sec->output_section->vma
			+ rent->sec->output_offset);

	      /* Compute the difference between the start of the section
		 with the relocation and the opd entry.  */
	      value -= value2;

	      /* The result becomes the addend of the relocation.  */
	      rel.r_addend = value;

	      /* The section symbol becomes the symbol for the dynamic
		 relocation.  */
	      dynindx
		= _bfd_elf_link_lookup_local_dynindx (info,
						      rent->sec->owner,
						      rent->sec_symndx);
	    }
	  else
	    rel.r_addend = rent->addend;

	  rel.r_info = ELF64_R_INFO (dynindx, rent->type);

	  loc = hppa_info->other_rel_sec->contents;
	  loc += (hppa_info->other_rel_sec->reloc_count++
		  * sizeof (Elf64_External_Rela));
	  bfd_elf64_swap_reloca_out (info->output_bfd, &rel, loc);
	}
    }

  return true;
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf64_hppa_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			     const asection *rel_sec ATTRIBUTE_UNUSED,
			     const Elf_Internal_Rela *rela)
{
  if (ELF64_R_SYM (rela->r_info) == STN_UNDEF)
    return reloc_class_relative;

  switch ((int) ELF64_R_TYPE (rela->r_info))
    {
    case R_PARISC_IPLT:
      return reloc_class_plt;
    case R_PARISC_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bool
elf64_hppa_finish_dynamic_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  struct elf64_hppa_link_hash_table *hppa_info;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  /* Finalize the contents of the .opd section.  */
  elf_link_hash_traverse (elf_hash_table (info),
			  elf64_hppa_finalize_opd,
			  info);

  elf_link_hash_traverse (elf_hash_table (info),
			  elf64_hppa_finalize_dynreloc,
			  info);

  /* Finalize the contents of the .dlt section.  */
  dynobj = elf_hash_table (info)->dynobj;
  /* Finalize the contents of the .dlt section.  */
  elf_link_hash_traverse (elf_hash_table (info),
			  elf64_hppa_finalize_dlt,
			  info);

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf64_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sdyn != NULL);

      dyncon = (Elf64_External_Dyn *) sdyn->contents;
      dynconend = (Elf64_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf64_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      break;

	    case DT_HP_LOAD_MAP:
	      /* Compute the absolute address of 16byte scratchpad area
		 for the dynamic linker.

		 By convention the linker script will allocate the scratchpad
		 area at the start of the .data section.  So all we have to
		 to is find the start of the .data section.  */
	      s = bfd_get_section_by_name (output_bfd, ".data");
	      if (!s)
		return false;
	      dyn.d_un.d_ptr = s->vma;
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTGOT:
	      /* HP's use PLTGOT to set the GOT register.  */
	      dyn.d_un.d_ptr = _bfd_get_gp_value (output_bfd);
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_JMPREL:
	      s = hppa_info->root.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = hppa_info->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_RELA:
	      s = hppa_info->other_rel_sec;
	      if (! s || ! s->size)
		s = hppa_info->dlt_rel_sec;
	      if (! s || ! s->size)
		s = hppa_info->opd_rel_sec;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_RELASZ:
	      s = hppa_info->other_rel_sec;
	      dyn.d_un.d_val = s->size;
	      s = hppa_info->dlt_rel_sec;
	      dyn.d_un.d_val += s->size;
	      s = hppa_info->opd_rel_sec;
	      dyn.d_un.d_val += s->size;
	      /* There is some question about whether or not the size of
		 the PLT relocs should be included here.  HP's tools do
		 it, so we'll emulate them.  */
	      s = hppa_info->root.srelplt;
	      dyn.d_un.d_val += s->size;
	      bfd_elf64_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    }
	}
    }

  return true;
}

/* Support for core dump NOTE sections.  */

static bool
elf64_hppa_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
      default:
	return false;

      case 760:		/* Linux/hppa */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 32);

	/* pr_reg */
	offset = 112;
	size = 640;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf64_hppa_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  char * command;
  int n;

  switch (note->descsz)
    {
    default:
      return false;

    case 136:		/* Linux/hppa elf_prpsinfo.  */
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 40, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 56, 80);
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */
  command = elf_tdata (abfd)->core->command;
  n = strlen (command);

  if (0 < n && command[n - 1] == ' ')
    command[n - 1] = '\0';

  return true;
}

/* Return the number of additional phdrs we will need.

   The generic ELF code only creates PT_PHDRs for executables.  The HP
   dynamic linker requires PT_PHDRs for dynamic libraries too.

   This routine indicates that the backend needs one additional program
   header for that case.

   Note we do not have access to the link info structure here, so we have
   to guess whether or not we are building a shared library based on the
   existence of a .interp section.  */

static int
elf64_hppa_additional_program_headers (bfd *abfd,
				struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;

  /* If we are creating a shared library, then we have to create a
     PT_PHDR segment.  HP's dynamic linker chokes without it.  */
  s = bfd_get_section_by_name (abfd, ".interp");
  if (! s)
    return 1;
  return 0;
}

static bool
elf64_hppa_allow_non_load_phdr (bfd *abfd ATTRIBUTE_UNUSED,
				const Elf_Internal_Phdr *phdr ATTRIBUTE_UNUSED,
				unsigned int count ATTRIBUTE_UNUSED)
{
  return true;
}

/* Allocate and initialize any program headers required by this
   specific backend.

   The generic ELF code only creates PT_PHDRs for executables.  The HP
   dynamic linker requires PT_PHDRs for dynamic libraries too.

   This allocates the PT_PHDR and initializes it in a manner suitable
   for the HP linker.

   Note we do not have access to the link info structure here, so we have
   to guess whether or not we are building a shared library based on the
   existence of a .interp section.  */

static bool
elf64_hppa_modify_segment_map (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_segment_map *m;

  m = elf_seg_map (abfd);
  if (info != NULL && !info->user_phdrs && m != NULL && m->p_type != PT_PHDR)
    {
      m = ((struct elf_segment_map *)
	   bfd_zalloc (abfd, (bfd_size_type) sizeof *m));
      if (m == NULL)
	return false;

      m->p_type = PT_PHDR;
      m->p_flags = PF_R | PF_X;
      m->p_flags_valid = 1;
      m->p_paddr_valid = 1;
      m->includes_phdrs = 1;

      m->next = elf_seg_map (abfd);
      elf_seg_map (abfd) = m;
    }

  for (m = elf_seg_map (abfd) ; m != NULL; m = m->next)
    if (m->p_type == PT_LOAD)
      {
	unsigned int i;

	for (i = 0; i < m->count; i++)
	  {
	    /* The code "hint" is not really a hint.  It is a requirement
	       for certain versions of the HP dynamic linker.  Worse yet,
	       it must be set even if the shared library does not have
	       any code in its "text" segment (thus the check for .hash
	       to catch this situation).  */
	    if (m->sections[i]->flags & SEC_CODE
		|| (strcmp (m->sections[i]->name, ".hash") == 0))
	      m->p_flags |= (PF_X | PF_HP_CODE);
	  }
      }

  return true;
}

/* Called when writing out an object file to decide the type of a
   symbol.  */
static int
elf64_hppa_elf_get_symbol_type (Elf_Internal_Sym *elf_sym,
				int type)
{
  if (ELF_ST_TYPE (elf_sym->st_info) == STT_PARISC_MILLI)
    return STT_PARISC_MILLI;
  else
    return type;
}

/* Support HP specific sections for core files.  */

static bool
elf64_hppa_section_from_phdr (bfd *abfd, Elf_Internal_Phdr *hdr, int sec_index,
			      const char *typename)
{
  if (hdr->p_type == PT_HP_CORE_KERNEL)
    {
      asection *sect;

      if (!_bfd_elf_make_section_from_phdr (abfd, hdr, sec_index, typename))
	return false;

      sect = bfd_make_section_anyway (abfd, ".kernel");
      if (sect == NULL)
	return false;
      sect->size = hdr->p_filesz;
      sect->filepos = hdr->p_offset;
      sect->flags = SEC_HAS_CONTENTS | SEC_READONLY;
      return true;
    }

  if (hdr->p_type == PT_HP_CORE_PROC)
    {
      int sig;

      if (bfd_seek (abfd, hdr->p_offset, SEEK_SET) != 0)
	return false;
      if (bfd_bread (&sig, 4, abfd) != 4)
	return false;

      elf_tdata (abfd)->core->signal = sig;

      if (!_bfd_elf_make_section_from_phdr (abfd, hdr, sec_index, typename))
	return false;

      /* GDB uses the ".reg" section to read register contents.  */
      return _bfd_elfcore_make_pseudosection (abfd, ".reg", hdr->p_filesz,
					      hdr->p_offset);
    }

  if (hdr->p_type == PT_HP_CORE_LOADABLE
      || hdr->p_type == PT_HP_CORE_STACK
      || hdr->p_type == PT_HP_CORE_MMF)
    hdr->p_type = PT_LOAD;

  return _bfd_elf_make_section_from_phdr (abfd, hdr, sec_index, typename);
}

/* Hook called by the linker routine which adds symbols from an object
   file.  HP's libraries define symbols with HP specific section
   indices, which we have to handle.  */

static bool
elf_hppa_add_symbol_hook (bfd *abfd,
			  struct bfd_link_info *info ATTRIBUTE_UNUSED,
			  Elf_Internal_Sym *sym,
			  const char **namep ATTRIBUTE_UNUSED,
			  flagword *flagsp ATTRIBUTE_UNUSED,
			  asection **secp,
			  bfd_vma *valp)
{
  unsigned int sec_index = sym->st_shndx;

  switch (sec_index)
    {
    case SHN_PARISC_ANSI_COMMON:
      *secp = bfd_make_section_old_way (abfd, ".PARISC.ansi.common");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;

    case SHN_PARISC_HUGE_COMMON:
      *secp = bfd_make_section_old_way (abfd, ".PARISC.huge.common");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    }

  return true;
}

static bool
elf_hppa_unmark_useless_dynamic_symbols (struct elf_link_hash_entry *h,
					 void *data)
{
  struct bfd_link_info *info = data;

  /* If we are not creating a shared library, and this symbol is
     referenced by a shared library but is not defined anywhere, then
     the generic code will warn that it is undefined.

     This behavior is undesirable on HPs since the standard shared
     libraries contain references to undefined symbols.

     So we twiddle the flags associated with such symbols so that they
     will not trigger the warning.  ?!? FIXME.  This is horribly fragile.

     Ultimately we should have better controls over the generic ELF BFD
     linker code.  */
  if (! bfd_link_relocatable (info)
      && info->unresolved_syms_in_shared_libs != RM_IGNORE
      && h->root.type == bfd_link_hash_undefined
      && h->ref_dynamic
      && !h->ref_regular)
    {
      h->ref_dynamic = 0;
      h->pointer_equality_needed = 1;
    }

  return true;
}

static bool
elf_hppa_remark_useless_dynamic_symbols (struct elf_link_hash_entry *h,
					 void *data)
{
  struct bfd_link_info *info = data;

  /* If we are not creating a shared library, and this symbol is
     referenced by a shared library but is not defined anywhere, then
     the generic code will warn that it is undefined.

     This behavior is undesirable on HPs since the standard shared
     libraries contain references to undefined symbols.

     So we twiddle the flags associated with such symbols so that they
     will not trigger the warning.  ?!? FIXME.  This is horribly fragile.

     Ultimately we should have better controls over the generic ELF BFD
     linker code.  */
  if (! bfd_link_relocatable (info)
      && info->unresolved_syms_in_shared_libs != RM_IGNORE
      && h->root.type == bfd_link_hash_undefined
      && !h->ref_dynamic
      && !h->ref_regular
      && h->pointer_equality_needed)
    {
      h->ref_dynamic = 1;
      h->pointer_equality_needed = 0;
    }

  return true;
}

static bool
elf_hppa_is_dynamic_loader_symbol (const char *name)
{
  return (! strcmp (name, "__CPU_REVISION")
	  || ! strcmp (name, "__CPU_KEYBITS_1")
	  || ! strcmp (name, "__SYSTEM_ID_D")
	  || ! strcmp (name, "__FPU_MODEL")
	  || ! strcmp (name, "__FPU_REVISION")
	  || ! strcmp (name, "__ARGC")
	  || ! strcmp (name, "__ARGV")
	  || ! strcmp (name, "__ENVP")
	  || ! strcmp (name, "__TLS_SIZE_D")
	  || ! strcmp (name, "__LOAD_INFO")
	  || ! strcmp (name, "__systab"));
}

/* Record the lowest address for the data and text segments.  */
static void
elf_hppa_record_segment_addrs (bfd *abfd,
			       asection *section,
			       void *data)
{
  struct elf64_hppa_link_hash_table *hppa_info = data;

  if ((section->flags & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
    {
      bfd_vma value;
      Elf_Internal_Phdr *p;

      p = _bfd_elf_find_segment_containing_section (abfd, section->output_section);
      BFD_ASSERT (p != NULL);
      value = p->p_vaddr;

      if (section->flags & SEC_READONLY)
	{
	  if (value < hppa_info->text_segment_base)
	    hppa_info->text_segment_base = value;
	}
      else
	{
	  if (value < hppa_info->data_segment_base)
	    hppa_info->data_segment_base = value;
	}
    }
}

/* Called after we have seen all the input files/sections, but before
   final symbol resolution and section placement has been determined.

   We use this hook to (possibly) provide a value for __gp, then we
   fall back to the generic ELF final link routine.  */

static bool
elf_hppa_final_link (bfd *abfd, struct bfd_link_info *info)
{
  struct stat buf;
  struct elf64_hppa_link_hash_table *hppa_info = hppa_link_hash_table (info);

  if (hppa_info == NULL)
    return false;

  if (! bfd_link_relocatable (info))
    {
      struct elf_link_hash_entry *gp;
      bfd_vma gp_val;

      /* The linker script defines a value for __gp iff it was referenced
	 by one of the objects being linked.  First try to find the symbol
	 in the hash table.  If that fails, just compute the value __gp
	 should have had.  */
      gp = elf_link_hash_lookup (elf_hash_table (info), "__gp", false,
				 false, false);

      if (gp)
	{

	  /* Adjust the value of __gp as we may want to slide it into the
	     .plt section so that the stubs can access PLT entries without
	     using an addil sequence.  */
	  gp->root.u.def.value += hppa_info->gp_offset;

	  gp_val = (gp->root.u.def.section->output_section->vma
		    + gp->root.u.def.section->output_offset
		    + gp->root.u.def.value);
	}
      else
	{
	  asection *sec;

	  /* First look for a .plt section.  If found, then __gp is the
	     address of the .plt + gp_offset.

	     If no .plt is found, then look for .dlt, .opd and .data (in
	     that order) and set __gp to the base address of whichever
	     section is found first.  */

	  sec = hppa_info->root.splt;
	  if (sec && ! (sec->flags & SEC_EXCLUDE))
	    gp_val = (sec->output_offset
		      + sec->output_section->vma
		      + hppa_info->gp_offset);
	  else
	    {
	      sec = hppa_info->dlt_sec;
	      if (!sec || (sec->flags & SEC_EXCLUDE))
		sec = hppa_info->opd_sec;
	      if (!sec || (sec->flags & SEC_EXCLUDE))
		sec = bfd_get_section_by_name (abfd, ".data");
	      if (!sec || (sec->flags & SEC_EXCLUDE))
		gp_val = 0;
	      else
		gp_val = sec->output_offset + sec->output_section->vma;
	    }
	}

      /* Install whatever value we found/computed for __gp.  */
      _bfd_set_gp_value (abfd, gp_val);
    }

  /* We need to know the base of the text and data segments so that we
     can perform SEGREL relocations.  We will record the base addresses
     when we encounter the first SEGREL relocation.  */
  hppa_info->text_segment_base = (bfd_vma)-1;
  hppa_info->data_segment_base = (bfd_vma)-1;

  /* HP's shared libraries have references to symbols that are not
     defined anywhere.  The generic ELF BFD linker code will complain
     about such symbols.

     So we detect the losing case and arrange for the flags on the symbol
     to indicate that it was never referenced.  This keeps the generic
     ELF BFD link code happy and appears to not create any secondary
     problems.  Ultimately we need a way to control the behavior of the
     generic ELF BFD link code better.  */
  elf_link_hash_traverse (elf_hash_table (info),
			  elf_hppa_unmark_useless_dynamic_symbols,
			  info);

  /* Invoke the regular ELF backend linker to do all the work.  */
  if (!bfd_elf_final_link (abfd, info))
    return false;

  elf_link_hash_traverse (elf_hash_table (info),
			  elf_hppa_remark_useless_dynamic_symbols,
			  info);

  /* If we're producing a final executable, sort the contents of the
     unwind section. */
  if (bfd_link_relocatable (info))
    return true;

  /* Do not attempt to sort non-regular files.  This is here
     especially for configure scripts and kernel builds which run
     tests with "ld [...] -o /dev/null".  */
  if (stat (bfd_get_filename (abfd), &buf) != 0
      || !S_ISREG(buf.st_mode))
    return true;

  return elf_hppa_sort_unwind (abfd);
}

/* Relocate the given INSN.  VALUE should be the actual value we want
   to insert into the instruction, ie by this point we should not be
   concerned with computing an offset relative to the DLT, PC, etc.
   Instead this routine is meant to handle the bit manipulations needed
   to insert the relocation into the given instruction.  */

static int
elf_hppa_relocate_insn (int insn, int sym_value, unsigned int r_type)
{
  switch (r_type)
    {
    /* This is any 22 bit branch.  In PA2.0 syntax it corresponds to
       the "B" instruction.  */
    case R_PARISC_PCREL22F:
    case R_PARISC_PCREL22C:
      return (insn & ~0x3ff1ffd) | re_assemble_22 (sym_value);

      /* This is any 12 bit branch.  */
    case R_PARISC_PCREL12F:
      return (insn & ~0x1ffd) | re_assemble_12 (sym_value);

    /* This is any 17 bit branch.  In PA2.0 syntax it also corresponds
       to the "B" instruction as well as BE.  */
    case R_PARISC_PCREL17F:
    case R_PARISC_DIR17F:
    case R_PARISC_DIR17R:
    case R_PARISC_PCREL17C:
    case R_PARISC_PCREL17R:
      return (insn & ~0x1f1ffd) | re_assemble_17 (sym_value);

    /* ADDIL or LDIL instructions.  */
    case R_PARISC_DLTREL21L:
    case R_PARISC_DLTIND21L:
    case R_PARISC_LTOFF_FPTR21L:
    case R_PARISC_PCREL21L:
    case R_PARISC_LTOFF_TP21L:
    case R_PARISC_DPREL21L:
    case R_PARISC_PLTOFF21L:
    case R_PARISC_DIR21L:
      return (insn & ~0x1fffff) | re_assemble_21 (sym_value);

    /* LDO and integer loads/stores with 14 bit displacements.  */
    case R_PARISC_DLTREL14R:
    case R_PARISC_DLTREL14F:
    case R_PARISC_DLTIND14R:
    case R_PARISC_DLTIND14F:
    case R_PARISC_LTOFF_FPTR14R:
    case R_PARISC_PCREL14R:
    case R_PARISC_PCREL14F:
    case R_PARISC_LTOFF_TP14R:
    case R_PARISC_LTOFF_TP14F:
    case R_PARISC_DPREL14R:
    case R_PARISC_DPREL14F:
    case R_PARISC_PLTOFF14R:
    case R_PARISC_PLTOFF14F:
    case R_PARISC_DIR14R:
    case R_PARISC_DIR14F:
      return (insn & ~0x3fff) | low_sign_unext (sym_value, 14);

    /* PA2.0W LDO and integer loads/stores with 16 bit displacements.  */
    case R_PARISC_LTOFF_FPTR16F:
    case R_PARISC_PCREL16F:
    case R_PARISC_LTOFF_TP16F:
    case R_PARISC_GPREL16F:
    case R_PARISC_PLTOFF16F:
    case R_PARISC_DIR16F:
    case R_PARISC_LTOFF16F:
      return (insn & ~0xffff) | re_assemble_16 (sym_value);

    /* Doubleword loads and stores with a 14 bit displacement.  */
    case R_PARISC_DLTREL14DR:
    case R_PARISC_DLTIND14DR:
    case R_PARISC_LTOFF_FPTR14DR:
    case R_PARISC_LTOFF_FPTR16DF:
    case R_PARISC_PCREL14DR:
    case R_PARISC_PCREL16DF:
    case R_PARISC_LTOFF_TP14DR:
    case R_PARISC_LTOFF_TP16DF:
    case R_PARISC_DPREL14DR:
    case R_PARISC_GPREL16DF:
    case R_PARISC_PLTOFF14DR:
    case R_PARISC_PLTOFF16DF:
    case R_PARISC_DIR14DR:
    case R_PARISC_DIR16DF:
    case R_PARISC_LTOFF16DF:
      return (insn & ~0x3ff1) | (((sym_value & 0x2000) >> 13)
				 | ((sym_value & 0x1ff8) << 1));

    /* Floating point single word load/store instructions.  */
    case R_PARISC_DLTREL14WR:
    case R_PARISC_DLTIND14WR:
    case R_PARISC_LTOFF_FPTR14WR:
    case R_PARISC_LTOFF_FPTR16WF:
    case R_PARISC_PCREL14WR:
    case R_PARISC_PCREL16WF:
    case R_PARISC_LTOFF_TP14WR:
    case R_PARISC_LTOFF_TP16WF:
    case R_PARISC_DPREL14WR:
    case R_PARISC_GPREL16WF:
    case R_PARISC_PLTOFF14WR:
    case R_PARISC_PLTOFF16WF:
    case R_PARISC_DIR16WF:
    case R_PARISC_DIR14WR:
    case R_PARISC_LTOFF16WF:
      return (insn & ~0x3ff9) | (((sym_value & 0x2000) >> 13)
				 | ((sym_value & 0x1ffc) << 1));

    default:
      return insn;
    }
}

/* Compute the value for a relocation (REL) during a final link stage,
   then insert the value into the proper location in CONTENTS.

   VALUE is a tentative value for the relocation and may be overridden
   and modified here based on the specific relocation to be performed.

   For example we do conversions for PC-relative branches in this routine
   or redirection of calls to external routines to stubs.

   The work of actually applying the relocation is left to a helper
   routine in an attempt to reduce the complexity and size of this
   function.  */

static bfd_reloc_status_type
elf_hppa_final_link_relocate (Elf_Internal_Rela *rel,
			      bfd *input_bfd,
			      bfd *output_bfd,
			      asection *input_section,
			      bfd_byte *contents,
			      bfd_vma value,
			      struct bfd_link_info *info,
			      asection *sym_sec,
			      struct elf_link_hash_entry *eh)
{
  struct elf64_hppa_link_hash_table *hppa_info = hppa_link_hash_table (info);
  struct elf64_hppa_link_hash_entry *hh = hppa_elf_hash_entry (eh);
  bfd_vma *local_offsets;
  Elf_Internal_Shdr *symtab_hdr;
  int insn;
  bfd_vma max_branch_offset = 0;
  bfd_vma offset = rel->r_offset;
  bfd_signed_vma addend = rel->r_addend;
  reloc_howto_type *howto = elf_hppa_howto_table + ELF_R_TYPE (rel->r_info);
  unsigned int r_symndx = ELF_R_SYM (rel->r_info);
  unsigned int r_type = howto->type;
  bfd_byte *hit_data = contents + offset;

  if (hppa_info == NULL)
    return bfd_reloc_notsupported;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  local_offsets = elf_local_got_offsets (input_bfd);
  insn = bfd_get_32 (input_bfd, hit_data);

  switch (r_type)
    {
    case R_PARISC_NONE:
      break;

    /* Basic function call support.

       Note for a call to a function defined in another dynamic library
       we want to redirect the call to a stub.  */

    /* PC relative relocs without an implicit offset.  */
    case R_PARISC_PCREL21L:
    case R_PARISC_PCREL14R:
    case R_PARISC_PCREL14F:
    case R_PARISC_PCREL14WR:
    case R_PARISC_PCREL14DR:
    case R_PARISC_PCREL16F:
    case R_PARISC_PCREL16WF:
    case R_PARISC_PCREL16DF:
      {
	/* If this is a call to a function defined in another dynamic
	   library, then redirect the call to the local stub for this
	   function.  */
	if (sym_sec == NULL || sym_sec->output_section == NULL)
	  value = (hh->stub_offset + hppa_info->stub_sec->output_offset
		   + hppa_info->stub_sec->output_section->vma);

	/* Turn VALUE into a proper PC relative address.  */
	value -= (offset + input_section->output_offset
		  + input_section->output_section->vma);

	/* Adjust for any field selectors.  */
	if (r_type == R_PARISC_PCREL21L)
	  value = hppa_field_adjust (value, -8 + addend, e_lsel);
	else if (r_type == R_PARISC_PCREL14F
		 || r_type == R_PARISC_PCREL16F
		 || r_type == R_PARISC_PCREL16WF
		 || r_type == R_PARISC_PCREL16DF)
	  value = hppa_field_adjust (value, -8 + addend, e_fsel);
	else
	  value = hppa_field_adjust (value, -8 + addend, e_rsel);

	/* Apply the relocation to the given instruction.  */
	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    case R_PARISC_PCREL12F:
    case R_PARISC_PCREL22F:
    case R_PARISC_PCREL17F:
    case R_PARISC_PCREL22C:
    case R_PARISC_PCREL17C:
    case R_PARISC_PCREL17R:
      {
	/* If this is a call to a function defined in another dynamic
	   library, then redirect the call to the local stub for this
	   function.  */
	if (sym_sec == NULL || sym_sec->output_section == NULL)
	  value = (hh->stub_offset + hppa_info->stub_sec->output_offset
		   + hppa_info->stub_sec->output_section->vma);

	/* Turn VALUE into a proper PC relative address.  */
	value -= (offset + input_section->output_offset
		  + input_section->output_section->vma);
	addend -= 8;

	if (r_type == (unsigned int) R_PARISC_PCREL22F)
	  max_branch_offset = (1 << (22-1)) << 2;
	else if (r_type == (unsigned int) R_PARISC_PCREL17F)
	  max_branch_offset = (1 << (17-1)) << 2;
	else if (r_type == (unsigned int) R_PARISC_PCREL12F)
	  max_branch_offset = (1 << (12-1)) << 2;

	/* Make sure we can reach the branch target.  */
	if (max_branch_offset != 0
	    && value + addend + max_branch_offset >= 2*max_branch_offset)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): cannot reach %s"),
	      input_bfd,
	      input_section,
	      (uint64_t) offset,
	      eh ? eh->root.root.string : "unknown");
	    bfd_set_error (bfd_error_bad_value);
	    return bfd_reloc_overflow;
	  }

	/* Adjust for any field selectors.  */
	if (r_type == R_PARISC_PCREL17R)
	  value = hppa_field_adjust (value, addend, e_rsel);
	else
	  value = hppa_field_adjust (value, addend, e_fsel);

	/* All branches are implicitly shifted by 2 places.  */
	value >>= 2;

	/* Apply the relocation to the given instruction.  */
	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    /* Indirect references to data through the DLT.  */
    case R_PARISC_DLTIND14R:
    case R_PARISC_DLTIND14F:
    case R_PARISC_DLTIND14DR:
    case R_PARISC_DLTIND14WR:
    case R_PARISC_DLTIND21L:
    case R_PARISC_LTOFF_FPTR14R:
    case R_PARISC_LTOFF_FPTR14DR:
    case R_PARISC_LTOFF_FPTR14WR:
    case R_PARISC_LTOFF_FPTR21L:
    case R_PARISC_LTOFF_FPTR16F:
    case R_PARISC_LTOFF_FPTR16WF:
    case R_PARISC_LTOFF_FPTR16DF:
    case R_PARISC_LTOFF_TP21L:
    case R_PARISC_LTOFF_TP14R:
    case R_PARISC_LTOFF_TP14F:
    case R_PARISC_LTOFF_TP14WR:
    case R_PARISC_LTOFF_TP14DR:
    case R_PARISC_LTOFF_TP16F:
    case R_PARISC_LTOFF_TP16WF:
    case R_PARISC_LTOFF_TP16DF:
    case R_PARISC_LTOFF16F:
    case R_PARISC_LTOFF16WF:
    case R_PARISC_LTOFF16DF:
      {
	bfd_vma off;

	/* If this relocation was against a local symbol, then we still
	   have not set up the DLT entry (it's not convenient to do so
	   in the "finalize_dlt" routine because it is difficult to get
	   to the local symbol's value).

	   So, if this is a local symbol (h == NULL), then we need to
	   fill in its DLT entry.

	   Similarly we may still need to set up an entry in .opd for
	   a local function which had its address taken.  */
	if (hh == NULL)
	  {
	    bfd_vma *local_opd_offsets, *local_dlt_offsets;

	    if (local_offsets == NULL)
	      abort ();

	    /* Now do .opd creation if needed.  */
	    if (r_type == R_PARISC_LTOFF_FPTR14R
		|| r_type == R_PARISC_LTOFF_FPTR14DR
		|| r_type == R_PARISC_LTOFF_FPTR14WR
		|| r_type == R_PARISC_LTOFF_FPTR21L
		|| r_type == R_PARISC_LTOFF_FPTR16F
		|| r_type == R_PARISC_LTOFF_FPTR16WF
		|| r_type == R_PARISC_LTOFF_FPTR16DF)
	      {
		local_opd_offsets = local_offsets + 2 * symtab_hdr->sh_info;
		off = local_opd_offsets[r_symndx];

		/* The last bit records whether we've already initialised
		   this local .opd entry.  */
		if ((off & 1) != 0)
		  {
		    BFD_ASSERT (off != (bfd_vma) -1);
		    off &= ~1;
		  }
		else
		  {
		    local_opd_offsets[r_symndx] |= 1;

		    /* The first two words of an .opd entry are zero.  */
		    memset (hppa_info->opd_sec->contents + off, 0, 16);

		    /* The next word is the address of the function.  */
		    bfd_put_64 (hppa_info->opd_sec->owner, value + addend,
				(hppa_info->opd_sec->contents + off + 16));

		    /* The last word is our local __gp value.  */
		    value = _bfd_get_gp_value (info->output_bfd);
		    bfd_put_64 (hppa_info->opd_sec->owner, value,
				(hppa_info->opd_sec->contents + off + 24));
		  }

		/* The DLT value is the address of the .opd entry.  */
		value = (off
			 + hppa_info->opd_sec->output_offset
			 + hppa_info->opd_sec->output_section->vma);
		addend = 0;
	      }

	    local_dlt_offsets = local_offsets;
	    off = local_dlt_offsets[r_symndx];

	    if ((off & 1) != 0)
	      {
		BFD_ASSERT (off != (bfd_vma) -1);
		off &= ~1;
	      }
	    else
	      {
		local_dlt_offsets[r_symndx] |= 1;
		bfd_put_64 (hppa_info->dlt_sec->owner,
			    value + addend,
			    hppa_info->dlt_sec->contents + off);
	      }
	  }
	else
	  off = hh->dlt_offset;

	/* We want the value of the DLT offset for this symbol, not
	   the symbol's actual address.  Note that __gp may not point
	   to the start of the DLT, so we have to compute the absolute
	   address, then subtract out the value of __gp.  */
	value = (off
		 + hppa_info->dlt_sec->output_offset
		 + hppa_info->dlt_sec->output_section->vma);
	value -= _bfd_get_gp_value (output_bfd);

	/* All DLTIND relocations are basically the same at this point,
	   except that we need different field selectors for the 21bit
	   version vs the 14bit versions.  */
	if (r_type == R_PARISC_DLTIND21L
	    || r_type == R_PARISC_LTOFF_FPTR21L
	    || r_type == R_PARISC_LTOFF_TP21L)
	  value = hppa_field_adjust (value, 0, e_lsel);
	else if (r_type == R_PARISC_DLTIND14F
		 || r_type == R_PARISC_LTOFF_FPTR16F
		 || r_type == R_PARISC_LTOFF_FPTR16WF
		 || r_type == R_PARISC_LTOFF_FPTR16DF
		 || r_type == R_PARISC_LTOFF16F
		 || r_type == R_PARISC_LTOFF16DF
		 || r_type == R_PARISC_LTOFF16WF
		 || r_type == R_PARISC_LTOFF_TP16F
		 || r_type == R_PARISC_LTOFF_TP16WF
		 || r_type == R_PARISC_LTOFF_TP16DF)
	  value = hppa_field_adjust (value, 0, e_fsel);
	else
	  value = hppa_field_adjust (value, 0, e_rsel);

	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    case R_PARISC_DLTREL14R:
    case R_PARISC_DLTREL14F:
    case R_PARISC_DLTREL14DR:
    case R_PARISC_DLTREL14WR:
    case R_PARISC_DLTREL21L:
    case R_PARISC_DPREL21L:
    case R_PARISC_DPREL14WR:
    case R_PARISC_DPREL14DR:
    case R_PARISC_DPREL14R:
    case R_PARISC_DPREL14F:
    case R_PARISC_GPREL16F:
    case R_PARISC_GPREL16WF:
    case R_PARISC_GPREL16DF:
      {
	/* Subtract out the global pointer value to make value a DLT
	   relative address.  */
	value -= _bfd_get_gp_value (output_bfd);

	/* All DLTREL relocations are basically the same at this point,
	   except that we need different field selectors for the 21bit
	   version vs the 14bit versions.  */
	if (r_type == R_PARISC_DLTREL21L
	    || r_type == R_PARISC_DPREL21L)
	  value = hppa_field_adjust (value, addend, e_lrsel);
	else if (r_type == R_PARISC_DLTREL14F
		 || r_type == R_PARISC_DPREL14F
		 || r_type == R_PARISC_GPREL16F
		 || r_type == R_PARISC_GPREL16WF
		 || r_type == R_PARISC_GPREL16DF)
	  value = hppa_field_adjust (value, addend, e_fsel);
	else
	  value = hppa_field_adjust (value, addend, e_rrsel);

	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    case R_PARISC_DIR21L:
    case R_PARISC_DIR17R:
    case R_PARISC_DIR17F:
    case R_PARISC_DIR14R:
    case R_PARISC_DIR14F:
    case R_PARISC_DIR14WR:
    case R_PARISC_DIR14DR:
    case R_PARISC_DIR16F:
    case R_PARISC_DIR16WF:
    case R_PARISC_DIR16DF:
      {
	/* All DIR relocations are basically the same at this point,
	   except that branch offsets need to be divided by four, and
	   we need different field selectors.  Note that we don't
	   redirect absolute calls to local stubs.  */

	if (r_type == R_PARISC_DIR21L)
	  value = hppa_field_adjust (value, addend, e_lrsel);
	else if (r_type == R_PARISC_DIR17F
		 || r_type == R_PARISC_DIR16F
		 || r_type == R_PARISC_DIR16WF
		 || r_type == R_PARISC_DIR16DF
		 || r_type == R_PARISC_DIR14F)
	  value = hppa_field_adjust (value, addend, e_fsel);
	else
	  value = hppa_field_adjust (value, addend, e_rrsel);

	if (r_type == R_PARISC_DIR17R || r_type == R_PARISC_DIR17F)
	  /* All branches are implicitly shifted by 2 places.  */
	  value >>= 2;

	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    case R_PARISC_PLTOFF21L:
    case R_PARISC_PLTOFF14R:
    case R_PARISC_PLTOFF14F:
    case R_PARISC_PLTOFF14WR:
    case R_PARISC_PLTOFF14DR:
    case R_PARISC_PLTOFF16F:
    case R_PARISC_PLTOFF16WF:
    case R_PARISC_PLTOFF16DF:
      {
	/* We want the value of the PLT offset for this symbol, not
	   the symbol's actual address.  Note that __gp may not point
	   to the start of the DLT, so we have to compute the absolute
	   address, then subtract out the value of __gp.  */
	value = (hh->plt_offset
		 + hppa_info->root.splt->output_offset
		 + hppa_info->root.splt->output_section->vma);
	value -= _bfd_get_gp_value (output_bfd);

	/* All PLTOFF relocations are basically the same at this point,
	   except that we need different field selectors for the 21bit
	   version vs the 14bit versions.  */
	if (r_type == R_PARISC_PLTOFF21L)
	  value = hppa_field_adjust (value, addend, e_lrsel);
	else if (r_type == R_PARISC_PLTOFF14F
		 || r_type == R_PARISC_PLTOFF16F
		 || r_type == R_PARISC_PLTOFF16WF
		 || r_type == R_PARISC_PLTOFF16DF)
	  value = hppa_field_adjust (value, addend, e_fsel);
	else
	  value = hppa_field_adjust (value, addend, e_rrsel);

	insn = elf_hppa_relocate_insn (insn, (int) value, r_type);
	break;
      }

    case R_PARISC_LTOFF_FPTR32:
      {
	/* FIXME: There used to be code here to create the FPTR itself if
	   the relocation was against a local symbol.  But the code could
	   never have worked.  If the assert below is ever triggered then
	   the code will need to be reinstated and fixed so that it does
	   what is needed.  */
	BFD_ASSERT (hh != NULL);

	/* We want the value of the DLT offset for this symbol, not
	   the symbol's actual address.  Note that __gp may not point
	   to the start of the DLT, so we have to compute the absolute
	   address, then subtract out the value of __gp.  */
	value = (hh->dlt_offset
		 + hppa_info->dlt_sec->output_offset
		 + hppa_info->dlt_sec->output_section->vma);
	value -= _bfd_get_gp_value (output_bfd);
	bfd_put_32 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    case R_PARISC_LTOFF_FPTR64:
    case R_PARISC_LTOFF_TP64:
      {
	/* We may still need to create the FPTR itself if it was for
	   a local symbol.  */
	if (eh == NULL && r_type == R_PARISC_LTOFF_FPTR64)
	  {
	    /* The first two words of an .opd entry are zero.  */
	    memset (hppa_info->opd_sec->contents + hh->opd_offset, 0, 16);

	    /* The next word is the address of the function.  */
	    bfd_put_64 (hppa_info->opd_sec->owner, value + addend,
			(hppa_info->opd_sec->contents
			 + hh->opd_offset + 16));

	    /* The last word is our local __gp value.  */
	    value = _bfd_get_gp_value (info->output_bfd);
	    bfd_put_64 (hppa_info->opd_sec->owner, value,
			hppa_info->opd_sec->contents + hh->opd_offset + 24);

	    /* The DLT value is the address of the .opd entry.  */
	    value = (hh->opd_offset
		     + hppa_info->opd_sec->output_offset
		     + hppa_info->opd_sec->output_section->vma);

	    bfd_put_64 (hppa_info->dlt_sec->owner,
			value,
			hppa_info->dlt_sec->contents + hh->dlt_offset);
	  }

	/* We want the value of the DLT offset for this symbol, not
	   the symbol's actual address.  Note that __gp may not point
	   to the start of the DLT, so we have to compute the absolute
	   address, then subtract out the value of __gp.  */
	value = (hh->dlt_offset
		 + hppa_info->dlt_sec->output_offset
		 + hppa_info->dlt_sec->output_section->vma);
	value -= _bfd_get_gp_value (output_bfd);
	bfd_put_64 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    case R_PARISC_DIR32:
      bfd_put_32 (input_bfd, value + addend, hit_data);
      return bfd_reloc_ok;

    case R_PARISC_DIR64:
      bfd_put_64 (input_bfd, value + addend, hit_data);
      return bfd_reloc_ok;

    case R_PARISC_GPREL64:
      /* Subtract out the global pointer value to make value a DLT
	 relative address.  */
      value -= _bfd_get_gp_value (output_bfd);

      bfd_put_64 (input_bfd, value + addend, hit_data);
      return bfd_reloc_ok;

    case R_PARISC_LTOFF64:
	/* We want the value of the DLT offset for this symbol, not
	   the symbol's actual address.  Note that __gp may not point
	   to the start of the DLT, so we have to compute the absolute
	   address, then subtract out the value of __gp.  */
      value = (hh->dlt_offset
	       + hppa_info->dlt_sec->output_offset
	       + hppa_info->dlt_sec->output_section->vma);
      value -= _bfd_get_gp_value (output_bfd);

      bfd_put_64 (input_bfd, value + addend, hit_data);
      return bfd_reloc_ok;

    case R_PARISC_PCREL32:
      {
	/* If this is a call to a function defined in another dynamic
	   library, then redirect the call to the local stub for this
	   function.  */
	if (sym_sec == NULL || sym_sec->output_section == NULL)
	  value = (hh->stub_offset + hppa_info->stub_sec->output_offset
		   + hppa_info->stub_sec->output_section->vma);

	/* Turn VALUE into a proper PC relative address.  */
	value -= (offset + input_section->output_offset
		  + input_section->output_section->vma);

	value += addend;
	value -= 8;
	bfd_put_32 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    case R_PARISC_PCREL64:
      {
	/* If this is a call to a function defined in another dynamic
	   library, then redirect the call to the local stub for this
	   function.  */
	if (sym_sec == NULL || sym_sec->output_section == NULL)
	  value = (hh->stub_offset + hppa_info->stub_sec->output_offset
		   + hppa_info->stub_sec->output_section->vma);

	/* Turn VALUE into a proper PC relative address.  */
	value -= (offset + input_section->output_offset
		  + input_section->output_section->vma);

	value += addend;
	value -= 8;
	bfd_put_64 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    case R_PARISC_FPTR64:
      {
	bfd_vma off;

	/* We may still need to create the FPTR itself if it was for
	   a local symbol.  */
	if (hh == NULL)
	  {
	    bfd_vma *local_opd_offsets;

	    if (local_offsets == NULL)
	      abort ();

	    local_opd_offsets = local_offsets + 2 * symtab_hdr->sh_info;
	    off = local_opd_offsets[r_symndx];

	    /* The last bit records whether we've already initialised
	       this local .opd entry.  */
	    if ((off & 1) != 0)
	      {
		BFD_ASSERT (off != (bfd_vma) -1);
		off &= ~1;
	      }
	    else
	      {
		/* The first two words of an .opd entry are zero.  */
		memset (hppa_info->opd_sec->contents + off, 0, 16);

		/* The next word is the address of the function.  */
		bfd_put_64 (hppa_info->opd_sec->owner, value + addend,
			    (hppa_info->opd_sec->contents + off + 16));

		/* The last word is our local __gp value.  */
		value = _bfd_get_gp_value (info->output_bfd);
		bfd_put_64 (hppa_info->opd_sec->owner, value,
			    hppa_info->opd_sec->contents + off + 24);
	      }
	  }
	else
	  off = hh->opd_offset;

	if (hh == NULL || hh->want_opd)
	  /* We want the value of the OPD offset for this symbol.  */
	  value = (off
		   + hppa_info->opd_sec->output_offset
		   + hppa_info->opd_sec->output_section->vma);
	else
	  /* We want the address of the symbol.  */
	  value += addend;

	bfd_put_64 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    case R_PARISC_SECREL32:
      if (sym_sec && sym_sec->output_section)
	value -= sym_sec->output_section->vma;
      bfd_put_32 (input_bfd, value + addend, hit_data);
      return bfd_reloc_ok;

    case R_PARISC_SEGREL32:
    case R_PARISC_SEGREL64:
      {
	/* If this is the first SEGREL relocation, then initialize
	   the segment base values.  */
	if (hppa_info->text_segment_base == (bfd_vma) -1)
	  bfd_map_over_sections (output_bfd, elf_hppa_record_segment_addrs,
				 hppa_info);

	/* VALUE holds the absolute address.  We want to include the
	   addend, then turn it into a segment relative address.

	   The segment is derived from SYM_SEC.  We assume that there are
	   only two segments of note in the resulting executable/shlib.
	   A readonly segment (.text) and a readwrite segment (.data).  */
	value += addend;

	if (sym_sec->flags & SEC_CODE)
	  value -= hppa_info->text_segment_base;
	else
	  value -= hppa_info->data_segment_base;

	if (r_type == R_PARISC_SEGREL32)
	  bfd_put_32 (input_bfd, value, hit_data);
	else
	  bfd_put_64 (input_bfd, value, hit_data);
	return bfd_reloc_ok;
      }

    /* Something we don't know how to handle.  */
    default:
      return bfd_reloc_notsupported;
    }

  /* Update the instruction word.  */
  bfd_put_32 (input_bfd, (bfd_vma) insn, hit_data);
  return bfd_reloc_ok;
}

/* Relocate an HPPA ELF section.  */

static int
elf64_hppa_relocate_section (bfd *output_bfd,
			   struct bfd_link_info *info,
			   bfd *input_bfd,
			   asection *input_section,
			   bfd_byte *contents,
			   Elf_Internal_Rela *relocs,
			   Elf_Internal_Sym *local_syms,
			   asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  struct elf64_hppa_link_hash_table *hppa_info;

  hppa_info = hppa_link_hash_table (info);
  if (hppa_info == NULL)
    return false;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto = elf_hppa_howto_table + ELF_R_TYPE (rel->r_info);
      unsigned long r_symndx;
      struct elf_link_hash_entry *eh;
      Elf_Internal_Sym *sym;
      asection *sym_sec;
      bfd_vma relocation;
      bfd_reloc_status_type r;

      r_type = ELF_R_TYPE (rel->r_info);
      if (r_type < 0 || r_type >= (int) R_PARISC_UNIMPLEMENTED)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      if (r_type == (unsigned int) R_PARISC_GNU_VTENTRY
	  || r_type == (unsigned int) R_PARISC_GNU_VTINHERIT)
	continue;

      /* This is a final link.  */
      r_symndx = ELF_R_SYM (rel->r_info);
      eh = NULL;
      sym = NULL;
      sym_sec = NULL;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* This is a local symbol, hh defaults to NULL.  */
	  sym = local_syms + r_symndx;
	  sym_sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sym_sec, rel);
	}
      else
	{
	  /* This is not a local symbol.  */
	  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);

	  /* It seems this can happen with erroneous or unsupported
	     input (mixing a.out and elf in an archive, for example.)  */
	  if (sym_hashes == NULL)
	    return false;

	  eh = sym_hashes[r_symndx - symtab_hdr->sh_info];

	  if (info->wrap_hash != NULL
	      && (input_section->flags & SEC_DEBUGGING) != 0)
	    eh = ((struct elf_link_hash_entry *)
		  unwrap_hash_lookup (info, input_bfd, &eh->root));

	  while (eh->root.type == bfd_link_hash_indirect
		 || eh->root.type == bfd_link_hash_warning)
	    eh = (struct elf_link_hash_entry *) eh->root.u.i.link;

	  relocation = 0;
	  if (eh->root.type == bfd_link_hash_defined
	      || eh->root.type == bfd_link_hash_defweak)
	    {
	      sym_sec = eh->root.u.def.section;
	      if (sym_sec != NULL
		  && sym_sec->output_section != NULL)
		relocation = (eh->root.u.def.value
			      + sym_sec->output_section->vma
			      + sym_sec->output_offset);
	    }
	  else if (eh->root.type == bfd_link_hash_undefweak)
	    ;
	  else if (info->unresolved_syms_in_objects == RM_IGNORE
		   && ELF_ST_VISIBILITY (eh->other) == STV_DEFAULT)
	    ;
	  else if (!bfd_link_relocatable (info)
		   && elf_hppa_is_dynamic_loader_symbol (eh->root.root.string))
	    continue;
	  else if (!bfd_link_relocatable (info))
	    {
	      bool err;

	      err = (info->unresolved_syms_in_objects == RM_DIAGNOSE
		     && !info->warn_unresolved_syms)
		|| ELF_ST_VISIBILITY (eh->other) != STV_DEFAULT;

	      info->callbacks->undefined_symbol
		(info, eh->root.root.string, input_bfd,
		 input_section, rel->r_offset, err);
	    }

	  if (!bfd_link_relocatable (info)
	      && relocation == 0
	      && eh->root.type != bfd_link_hash_defined
	      && eh->root.type != bfd_link_hash_defweak
	      && eh->root.type != bfd_link_hash_undefweak)
	    {
	      if (info->unresolved_syms_in_objects == RM_IGNORE
		  && ELF_ST_VISIBILITY (eh->other) == STV_DEFAULT
		  && eh->type == STT_PARISC_MILLI)
		info->callbacks->undefined_symbol
		  (info, eh_name (eh), input_bfd,
		   input_section, rel->r_offset, false);
	    }
	}

      if (sym_sec != NULL && discarded_section (sym_sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      r = elf_hppa_final_link_relocate (rel, input_bfd, output_bfd,
					input_section, contents,
					relocation, info, sym_sec,
					eh);

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    default:
	      abort ();
	    case bfd_reloc_overflow:
	      {
		const char *sym_name;

		if (eh != NULL)
		  sym_name = NULL;
		else
		  {
		    sym_name = bfd_elf_string_from_elf_section (input_bfd,
								symtab_hdr->sh_link,
								sym->st_name);
		    if (sym_name == NULL)
		      return false;
		    if (*sym_name == '\0')
		      sym_name = bfd_section_name (sym_sec);
		  }

		(*info->callbacks->reloc_overflow)
		  (info, (eh ? &eh->root : NULL), sym_name, howto->name,
		   (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      }
	      break;
	    }
	}
    }
  return true;
}

static const struct bfd_elf_special_section elf64_hppa_special_sections[] =
{
  { STRING_COMMA_LEN (".tbss"),	 0, SHT_NOBITS, SHF_ALLOC + SHF_WRITE + SHF_HP_TLS },
  { STRING_COMMA_LEN (".fini"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".init"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".plt"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_PARISC_SHORT },
  { STRING_COMMA_LEN (".dlt"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_PARISC_SHORT },
  { STRING_COMMA_LEN (".sdata"), 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_PARISC_SHORT },
  { STRING_COMMA_LEN (".sbss"),	 0, SHT_NOBITS, SHF_ALLOC + SHF_WRITE + SHF_PARISC_SHORT },
  { NULL,		     0,	 0, 0,		  0 }
};

/* The hash bucket size is the standard one, namely 4.  */

const struct elf_size_info hppa64_elf_size_info =
{
  sizeof (Elf64_External_Ehdr),
  sizeof (Elf64_External_Phdr),
  sizeof (Elf64_External_Shdr),
  sizeof (Elf64_External_Rel),
  sizeof (Elf64_External_Rela),
  sizeof (Elf64_External_Sym),
  sizeof (Elf64_External_Dyn),
  sizeof (Elf_External_Note),
  4,
  1,
  64, 3,
  ELFCLASS64, EV_CURRENT,
  bfd_elf64_write_out_phdrs,
  bfd_elf64_write_shdrs_and_ehdr,
  bfd_elf64_checksum_contents,
  bfd_elf64_write_relocs,
  bfd_elf64_swap_symbol_in,
  bfd_elf64_swap_symbol_out,
  bfd_elf64_slurp_reloc_table,
  bfd_elf64_slurp_symbol_table,
  bfd_elf64_swap_dyn_in,
  bfd_elf64_swap_dyn_out,
  bfd_elf64_swap_reloc_in,
  bfd_elf64_swap_reloc_out,
  bfd_elf64_swap_reloca_in,
  bfd_elf64_swap_reloca_out
};

#define TARGET_BIG_SYM			hppa_elf64_vec
#define TARGET_BIG_NAME			"elf64-hppa"
#define ELF_ARCH			bfd_arch_hppa
#define ELF_TARGET_ID			HPPA64_ELF_DATA
#define ELF_MACHINE_CODE		EM_PARISC
/* This is not strictly correct.  The maximum page size for PA2.0 is
   64M.  But everything still uses 4k.  */
#define ELF_MAXPAGESIZE			0x1000
#define ELF_OSABI			ELFOSABI_HPUX

#define bfd_elf64_bfd_reloc_type_lookup elf_hppa_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup elf_hppa_reloc_name_lookup
#define bfd_elf64_bfd_is_local_label_name       elf_hppa_is_local_label_name
#define elf_info_to_howto		elf_hppa_info_to_howto
#define elf_info_to_howto_rel		elf_hppa_info_to_howto_rel

#define elf_backend_section_from_shdr	elf64_hppa_section_from_shdr
#define elf_backend_object_p		elf64_hppa_object_p
#define elf_backend_final_write_processing \
					elf_hppa_final_write_processing
#define elf_backend_fake_sections	elf_hppa_fake_sections
#define elf_backend_add_symbol_hook	elf_hppa_add_symbol_hook

#define elf_backend_relocate_section	elf_hppa_relocate_section

#define bfd_elf64_bfd_final_link	elf_hppa_final_link

#define elf_backend_create_dynamic_sections \
					elf64_hppa_create_dynamic_sections
#define elf_backend_init_file_header	elf64_hppa_init_file_header

#define elf_backend_omit_section_dynsym _bfd_elf_omit_section_dynsym_all

#define elf_backend_adjust_dynamic_symbol \
					elf64_hppa_adjust_dynamic_symbol

#define elf_backend_size_dynamic_sections \
					elf64_hppa_size_dynamic_sections

#define elf_backend_finish_dynamic_symbol \
					elf64_hppa_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					elf64_hppa_finish_dynamic_sections
#define elf_backend_grok_prstatus	elf64_hppa_grok_prstatus
#define elf_backend_grok_psinfo		elf64_hppa_grok_psinfo

/* Stuff for the BFD linker: */
#define bfd_elf64_bfd_link_hash_table_create \
	elf64_hppa_hash_table_create

#define elf_backend_check_relocs \
	elf64_hppa_check_relocs

#define elf_backend_size_info \
  hppa64_elf_size_info

#define elf_backend_additional_program_headers \
	elf64_hppa_additional_program_headers

#define elf_backend_modify_segment_map \
	elf64_hppa_modify_segment_map

#define elf_backend_allow_non_load_phdr \
	elf64_hppa_allow_non_load_phdr

#define elf_backend_link_output_symbol_hook \
	elf64_hppa_link_output_symbol_hook

#define elf_backend_want_got_plt	0
#define elf_backend_plt_readonly	0
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size     0
#define elf_backend_type_change_ok	true
#define elf_backend_get_symbol_type	elf64_hppa_elf_get_symbol_type
#define elf_backend_reloc_type_class	elf64_hppa_reloc_type_class
#define elf_backend_rela_normal		1
#define elf_backend_special_sections	elf64_hppa_special_sections
#define elf_backend_action_discarded	elf_hppa_action_discarded
#define elf_backend_section_from_phdr   elf64_hppa_section_from_phdr

#define elf64_bed			elf64_hppa_hpux_bed

#include "elf64-target.h"

#undef TARGET_BIG_SYM
#define TARGET_BIG_SYM			hppa_elf64_linux_vec
#undef TARGET_BIG_NAME
#define TARGET_BIG_NAME			"elf64-hppa-linux"
#undef ELF_OSABI
#define ELF_OSABI			ELFOSABI_GNU
#undef elf64_bed
#define elf64_bed			elf64_hppa_linux_bed
#undef elf_backend_special_sections
#define elf_backend_special_sections	(elf64_hppa_special_sections + 1)

#include "elf64-target.h"
