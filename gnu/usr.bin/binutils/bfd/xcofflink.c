/* POWER/PowerPC XCOFF linker support.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>, Cygnus Support.

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
#include "coff/internal.h"
#include "coff/xcoff.h"
#include "libcoff.h"
#include "libxcoff.h"
#include "libiberty.h"
#include "xcofflink.h"

/* This file holds the XCOFF linker code.  */

#undef  STRING_SIZE_SIZE
#define STRING_SIZE_SIZE 4

/* The list of import files.  */

struct xcoff_import_file
{
  /* The next entry in the list.  */
  struct xcoff_import_file *next;
  /* The path.  */
  const char *path;
  /* The file name.  */
  const char *file;
  /* The member name.  */
  const char *member;
};

/* Information we keep for each section in the output file during the
   final link phase.  */

struct xcoff_link_section_info
{
  /* The relocs to be output.  */
  struct internal_reloc *relocs;
  /* For each reloc against a global symbol whose index was not known
     when the reloc was handled, the global hash table entry.  */
  struct xcoff_link_hash_entry **rel_hashes;
  /* If there is a TOC relative reloc against a global symbol, and the
     index of the TOC symbol is not known when the reloc was handled,
     an entry is added to this linked list.  This is not an array,
     like rel_hashes, because this case is quite uncommon.  */
  struct xcoff_toc_rel_hash
  {
    struct xcoff_toc_rel_hash *next;
    struct xcoff_link_hash_entry *h;
    struct internal_reloc *rel;
  } *toc_rel_hashes;
};

/* Information that the XCOFF linker collects about an archive.  */
struct xcoff_archive_info
{
  /* The archive described by this entry.  */
  bfd *archive;

  /* The import path and import filename to use when referring to
     this archive in the .loader section.  */
  const char *imppath;
  const char *impfile;

  /* True if the archive contains a dynamic object.  */
  unsigned int contains_shared_object_p : 1;

  /* True if the previous field is valid.  */
  unsigned int know_contains_shared_object_p : 1;
};

struct xcoff_link_hash_table
{
  struct bfd_link_hash_table root;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Info passed by the linker.  */
  struct bfd_xcoff_link_params *params;

  /* The .debug string hash table.  We need to compute this while
     reading the input files, so that we know how large the .debug
     section will be before we assign section positions.  */
  struct bfd_strtab_hash *debug_strtab;

  /* The .debug section we will use for the final output.  */
  asection *debug_section;

  /* The .loader section we will use for the final output.  */
  asection *loader_section;

  /* The structure holding information about the .loader section.  */
  struct xcoff_loader_info ldinfo;

  /* The .loader section header.  */
  struct internal_ldhdr ldhdr;

  /* The .gl section we use to hold global linkage code.  */
  asection *linkage_section;

  /* The .tc section we use to hold toc entries we build for global
     linkage code.  */
  asection *toc_section;

  /* The .ds section we use to hold function descriptors which we
     create for exported symbols.  */
  asection *descriptor_section;

  /* The list of import files.  */
  struct xcoff_import_file *imports;

  /* Required alignment of sections within the output file.  */
  unsigned long file_align;

  /* Whether the .text section must be read-only.  */
  bool textro;

  /* Whether -brtl was specified.  */
  bool rtld;

  /* Whether garbage collection was done.  */
  bool gc;

  /* A linked list of symbols for which we have size information.  */
  struct xcoff_link_size_list
  {
    struct xcoff_link_size_list *next;
    struct xcoff_link_hash_entry *h;
    bfd_size_type size;
  }
  *size_list;

  /* Information about archives.  */
  htab_t archive_info;

  /* Magic sections: _text, _etext, _data, _edata, _end, end. */
  asection *special_sections[XCOFF_NUMBER_OF_SPECIAL_SECTIONS];
};

/* Information that we pass around while doing the final link step.  */

struct xcoff_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Hash table for long symbol names.  */
  struct bfd_strtab_hash *strtab;
  /* Array of information kept for each output section, indexed by the
     target_index field.  */
  struct xcoff_link_section_info *section_info;
  /* Symbol index of last C_FILE symbol (-1 if none).  */
  long last_file_index;
  /* Contents of last C_FILE symbol.  */
  struct internal_syment last_file;
  /* Symbol index of TOC symbol.  */
  long toc_symindx;
  /* Start of .loader symbols.  */
  bfd_byte *ldsym;
  /* Next .loader reloc to swap out.  */
  bfd_byte *ldrel;
  /* File position of start of line numbers.  */
  file_ptr line_filepos;
  /* Buffer large enough to hold swapped symbols of any input file.  */
  struct internal_syment *internal_syms;
  /* Buffer large enough to hold output indices of symbols of any
     input file.  */
  long *sym_indices;
  /* Buffer large enough to hold output symbols for any input file.  */
  bfd_byte *outsyms;
  /* Buffer large enough to hold external line numbers for any input
     section.  */
  bfd_byte *linenos;
  /* Buffer large enough to hold any input section.  */
  bfd_byte *contents;
  /* Buffer large enough to hold external relocs of any input section.  */
  bfd_byte *external_relocs;
};

#define xcoff_stub_hash_entry(ent)		\
  ((struct xcoff_stub_hash_entry *)(ent))

#define xcoff_stub_hash_lookup(table, string, create, copy)	\
  ((struct xcoff_stub_hash_entry *)				\
   bfd_hash_lookup ((table), (string), (create), (copy)))

static bool xcoff_mark (struct bfd_link_info *, asection *);



/* Routines to read XCOFF dynamic information.  This don't really
   belong here, but we already have the ldsym manipulation routines
   here.  */

/* Read the contents of a section.  */

static bfd_byte *
xcoff_get_section_contents (bfd *abfd, asection *sec)
{
  if (coff_section_data (abfd, sec) == NULL)
    {
      size_t amt = sizeof (struct coff_section_tdata);

      sec->used_by_bfd = bfd_zalloc (abfd, amt);
      if (sec->used_by_bfd == NULL)
       return NULL;
    }

  bfd_byte *contents = coff_section_data (abfd, sec)->contents;
  if (contents == NULL)
    {
      if (bfd_malloc_and_get_section (abfd, sec, &contents))
	coff_section_data (abfd, sec)->contents = contents;
      else
	{
	  free (contents);
	  contents = NULL;
	}
    }

  return contents;
}

/* Get the size required to hold the dynamic symbols.  */

long
_bfd_xcoff_get_dynamic_symtab_upper_bound (bfd *abfd)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return -1;

  bfd_xcoff_swap_ldhdr_in (abfd, (void *) contents, &ldhdr);

  return (ldhdr.l_nsyms + 1) * sizeof (asymbol *);
}

/* Get the dynamic symbols.  */

long
_bfd_xcoff_canonicalize_dynamic_symtab (bfd *abfd, asymbol **psyms)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  const char *strings;
  bfd_byte *elsym, *elsymend;
  coff_symbol_type *symbuf;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return -1;

  bfd_xcoff_swap_ldhdr_in (abfd, contents, &ldhdr);

  strings = (char *) contents + ldhdr.l_stoff;

  symbuf = bfd_zalloc (abfd, ldhdr.l_nsyms * sizeof (* symbuf));
  if (symbuf == NULL)
    return -1;

  elsym = contents + bfd_xcoff_loader_symbol_offset(abfd, &ldhdr);

  elsymend = elsym + ldhdr.l_nsyms * bfd_xcoff_ldsymsz(abfd);
  for (; elsym < elsymend; elsym += bfd_xcoff_ldsymsz(abfd), symbuf++, psyms++)
    {
      struct internal_ldsym ldsym;

      bfd_xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      symbuf->symbol.the_bfd = abfd;

      if (ldsym._l._l_l._l_zeroes == 0)
	symbuf->symbol.name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  char *c;

	  c = bfd_alloc (abfd, (bfd_size_type) SYMNMLEN + 1);
	  if (c == NULL)
	    return -1;
	  memcpy (c, ldsym._l._l_name, SYMNMLEN);
	  c[SYMNMLEN] = '\0';
	  symbuf->symbol.name = c;
	}

      if (ldsym.l_smclas == XMC_XO)
	symbuf->symbol.section = bfd_abs_section_ptr;
      else
	symbuf->symbol.section = coff_section_from_bfd_index (abfd,
							      ldsym.l_scnum);
      symbuf->symbol.value = ldsym.l_value - symbuf->symbol.section->vma;

      symbuf->symbol.flags = BSF_NO_FLAGS;
      if ((ldsym.l_smtype & L_EXPORT) != 0)
	{
	  if ((ldsym.l_smtype & L_WEAK) != 0)
	    symbuf->symbol.flags |= BSF_WEAK;
	  else
	    symbuf->symbol.flags |= BSF_GLOBAL;
	}

      /* FIXME: We have no way to record the other information stored
	 with the loader symbol.  */
      *psyms = (asymbol *) symbuf;
    }

  *psyms = NULL;

  return ldhdr.l_nsyms;
}

/* Get the size required to hold the dynamic relocs.  */

long
_bfd_xcoff_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return -1;

  bfd_xcoff_swap_ldhdr_in (abfd, (struct external_ldhdr *) contents, &ldhdr);

  return (ldhdr.l_nreloc + 1) * sizeof (arelent *);
}

/* Get the dynamic relocs.  */

long
_bfd_xcoff_canonicalize_dynamic_reloc (bfd *abfd,
				       arelent **prelocs,
				       asymbol **syms)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  arelent *relbuf;
  bfd_byte *elrel, *elrelend;

  if ((abfd->flags & DYNAMIC) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return -1;

  bfd_xcoff_swap_ldhdr_in (abfd, contents, &ldhdr);

  relbuf = bfd_alloc (abfd, ldhdr.l_nreloc * sizeof (arelent));
  if (relbuf == NULL)
    return -1;

  elrel = contents + bfd_xcoff_loader_reloc_offset(abfd, &ldhdr);

  elrelend = elrel + ldhdr.l_nreloc * bfd_xcoff_ldrelsz(abfd);
  for (; elrel < elrelend; elrel += bfd_xcoff_ldrelsz(abfd), relbuf++,
	 prelocs++)
    {
      struct internal_ldrel ldrel;

      bfd_xcoff_swap_ldrel_in (abfd, elrel, &ldrel);

      if (ldrel.l_symndx >= 3)
	relbuf->sym_ptr_ptr = syms + (ldrel.l_symndx - 3);
      else
	{
	  const char *name;
	  asection *sec;

	  switch (ldrel.l_symndx)
	    {
	    case 0:
	      name = ".text";
	      break;
	    case 1:
	      name = ".data";
	      break;
	    case 2:
	      name = ".bss";
	      break;
	    default:
	      abort ();
	      break;
	    }

	  sec = bfd_get_section_by_name (abfd, name);
	  if (sec == NULL)
	    {
	      bfd_set_error (bfd_error_bad_value);
	      return -1;
	    }

	  relbuf->sym_ptr_ptr = sec->symbol_ptr_ptr;
	}

      relbuf->address = ldrel.l_vaddr;
      relbuf->addend = 0;

      /* Most dynamic relocs have the same type.  FIXME: This is only
	 correct if ldrel.l_rtype == 0.  In other cases, we should use
	 a different howto.  */
      relbuf->howto = bfd_xcoff_dynamic_reloc_howto(abfd);

      /* FIXME: We have no way to record the l_rsecnm field.  */

      *prelocs = relbuf;
    }

  *prelocs = NULL;

  return ldhdr.l_nreloc;
}

/* Hash functions for xcoff_link_hash_table's archive_info.  */

static hashval_t
xcoff_archive_info_hash (const void *data)
{
  const struct xcoff_archive_info *info;

  info = (const struct xcoff_archive_info *) data;
  return htab_hash_pointer (info->archive);
}

static int
xcoff_archive_info_eq (const void *data1, const void *data2)
{
  const struct xcoff_archive_info *info1;
  const struct xcoff_archive_info *info2;

  info1 = (const struct xcoff_archive_info *) data1;
  info2 = (const struct xcoff_archive_info *) data2;
  return info1->archive == info2->archive;
}

/* Return information about archive ARCHIVE.  Return NULL on error.  */

static struct xcoff_archive_info *
xcoff_get_archive_info (struct bfd_link_info *info, bfd *archive)
{
  struct xcoff_link_hash_table *htab;
  struct xcoff_archive_info *entryp, entry;
  void **slot;

  htab = xcoff_hash_table (info);
  entry.archive = archive;
  slot = htab_find_slot (htab->archive_info, &entry, INSERT);
  if (!slot)
    return NULL;

  entryp = *slot;
  if (!entryp)
    {
      entryp = bfd_zalloc (info->output_bfd, sizeof (entry));
      if (!entryp)
	return NULL;

      entryp->archive = archive;
      *slot = entryp;
    }
  return entryp;
}


/* Initialize an entry in the stub hash table.  */
static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct xcoff_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct xcoff_stub_hash_entry *hsh;

      /* Initialize the local fields.  */
      hsh = (struct xcoff_stub_hash_entry *) entry;
      hsh->stub_type = xcoff_stub_none;
      hsh->hcsect = NULL;
      hsh->stub_offset = 0;
      hsh->target_section = NULL;
      hsh->htarget = NULL;
    }

  return entry;
}

/* Routine to create an entry in an XCOFF link hash table.  */

static struct bfd_hash_entry *
xcoff_link_hash_newfunc (struct bfd_hash_entry *entry,
			 struct bfd_hash_table *table,
			 const char *string)
{
  struct xcoff_link_hash_entry *ret = (struct xcoff_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table, sizeof (* ret));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = ((struct xcoff_link_hash_entry *)
	 _bfd_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				 table, string));
  if (ret != NULL)
    {
      /* Set local fields.  */
      ret->indx = -1;
      ret->toc_section = NULL;
      ret->u.toc_indx = -1;
      ret->descriptor = NULL;
      ret->ldsym = NULL;
      ret->ldindx = -1;
      ret->flags = 0;
      ret->smclas = XMC_UA;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Destroy an XCOFF link hash table.  */

static void
_bfd_xcoff_bfd_link_hash_table_free (bfd *obfd)
{
  struct xcoff_link_hash_table *ret;

  ret = (struct xcoff_link_hash_table *) obfd->link.hash;
  if (ret->archive_info)
    htab_delete (ret->archive_info);
  if (ret->debug_strtab)
    _bfd_stringtab_free (ret->debug_strtab);

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_generic_link_hash_table_free (obfd);
}

/* Create an XCOFF link hash table.  */

struct bfd_link_hash_table *
_bfd_xcoff_bfd_link_hash_table_create (bfd *abfd)
{
  struct xcoff_link_hash_table *ret;
  bool isxcoff64 = false;
  size_t amt = sizeof (* ret);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;
  if (!_bfd_link_hash_table_init (&ret->root, abfd, xcoff_link_hash_newfunc,
				  sizeof (struct xcoff_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }

  /* Init the stub hash table too.  */
  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct xcoff_stub_hash_entry)))
    {
      _bfd_xcoff_bfd_link_hash_table_free (abfd);
      return NULL;
    }

  isxcoff64 = bfd_coff_debug_string_prefix_length (abfd) == 4;

  ret->debug_strtab = _bfd_xcoff_stringtab_init (isxcoff64);
  ret->archive_info = htab_create (37, xcoff_archive_info_hash,
				   xcoff_archive_info_eq, NULL);
  if (!ret->debug_strtab || !ret->archive_info)
    {
      _bfd_xcoff_bfd_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.hash_table_free = _bfd_xcoff_bfd_link_hash_table_free;

  /* The linker will always generate a full a.out header.  We need to
     record that fact now, before the sizeof_headers routine could be
     called.  */
  xcoff_data (abfd)->full_aouthdr = true;

  return &ret->root;
}

/* Read internal relocs for an XCOFF csect.  This is a wrapper around
   _bfd_coff_read_internal_relocs which tries to take advantage of any
   relocs which may have been cached for the enclosing section.  */

static struct internal_reloc *
xcoff_read_internal_relocs (bfd *abfd,
			    asection *sec,
			    bool cache,
			    bfd_byte *external_relocs,
			    bool require_internal,
			    struct internal_reloc *internal_relocs)
{
  if (coff_section_data (abfd, sec) != NULL
      && coff_section_data (abfd, sec)->relocs == NULL
      && xcoff_section_data (abfd, sec) != NULL)
    {
      asection *enclosing;

      enclosing = xcoff_section_data (abfd, sec)->enclosing;

      if (enclosing != NULL
	  && (coff_section_data (abfd, enclosing) == NULL
	      || coff_section_data (abfd, enclosing)->relocs == NULL)
	  && cache
	  && enclosing->reloc_count > 0)
	{
	  if (_bfd_coff_read_internal_relocs (abfd, enclosing, true,
					      external_relocs, false, NULL)
	      == NULL)
	    return NULL;
	}

      if (enclosing != NULL
	  && coff_section_data (abfd, enclosing) != NULL
	  && coff_section_data (abfd, enclosing)->relocs != NULL)
	{
	  size_t off;

	  off = ((sec->rel_filepos - enclosing->rel_filepos)
		 / bfd_coff_relsz (abfd));

	  if (! require_internal)
	    return coff_section_data (abfd, enclosing)->relocs + off;
	  memcpy (internal_relocs,
		  coff_section_data (abfd, enclosing)->relocs + off,
		  sec->reloc_count * sizeof (struct internal_reloc));
	  return internal_relocs;
	}
    }

  return _bfd_coff_read_internal_relocs (abfd, sec, cache, external_relocs,
					 require_internal, internal_relocs);
}

/* Split FILENAME into an import path and an import filename,
   storing them in *IMPPATH and *IMPFILE respectively.  */

bool
bfd_xcoff_split_import_path (bfd *abfd, const char *filename,
			     const char **imppath, const char **impfile)
{
  const char *base;
  size_t length;
  char *path;

  base = lbasename (filename);
  length = base - filename;
  if (length == 0)
    /* The filename has no directory component, so use an empty path.  */
    *imppath = "";
  else if (length == 1)
    /* The filename is in the root directory.  */
    *imppath = "/";
  else
    {
      /* Extract the (non-empty) directory part.  Note that we don't
	 need to strip duplicate directory separators from any part
	 of the string; the native linker doesn't do that either.  */
      path = bfd_alloc (abfd, length);
      if (path == NULL)
	return false;
      memcpy (path, filename, length - 1);
      path[length - 1] = 0;
      *imppath = path;
    }
  *impfile = base;
  return true;
}

/* Set ARCHIVE's import path as though its filename had been given
   as FILENAME.  */

bool
bfd_xcoff_set_archive_import_path (struct bfd_link_info *info,
				   bfd *archive, const char *filename)
{
  struct xcoff_archive_info *archive_info;

  archive_info = xcoff_get_archive_info (info, archive);
  return (archive_info != NULL
	  && bfd_xcoff_split_import_path (archive, filename,
					  &archive_info->imppath,
					  &archive_info->impfile));
}

/* H is an imported symbol.  Set the import module's path, file and member
   to IMPATH, IMPFILE and IMPMEMBER respectively.  All three are null if
   no specific import module is specified.  */

static bool
xcoff_set_import_path (struct bfd_link_info *info,
		       struct xcoff_link_hash_entry *h,
		       const char *imppath, const char *impfile,
		       const char *impmember)
{
  unsigned int c;
  struct xcoff_import_file **pp;

  /* We overload the ldindx field to hold the l_ifile value for this
     symbol.  */
  BFD_ASSERT (h->ldsym == NULL);
  BFD_ASSERT ((h->flags & XCOFF_BUILT_LDSYM) == 0);
  if (imppath == NULL)
    h->ldindx = -1;
  else
    {
      /* We start c at 1 because the first entry in the import list is
	 reserved for the library search path.  */
      for (pp = &xcoff_hash_table (info)->imports, c = 1;
	   *pp != NULL;
	   pp = &(*pp)->next, ++c)
	{
	  if (filename_cmp ((*pp)->path, imppath) == 0
	      && filename_cmp ((*pp)->file, impfile) == 0
	      && filename_cmp ((*pp)->member, impmember) == 0)
	    break;
	}

      if (*pp == NULL)
	{
	  struct xcoff_import_file *n;
	  size_t amt = sizeof (*n);

	  n = bfd_alloc (info->output_bfd, amt);
	  if (n == NULL)
	    return false;
	  n->next = NULL;
	  n->path = imppath;
	  n->file = impfile;
	  n->member = impmember;
	  *pp = n;
	}
      h->ldindx = c;
    }
  return true;
}

/* H is the bfd symbol associated with exported .loader symbol LDSYM.
   Return true if LDSYM defines H.  */

static bool
xcoff_dynamic_definition_p (struct xcoff_link_hash_entry *h,
			    struct internal_ldsym *ldsym)
{
  /* If we didn't know about H before processing LDSYM, LDSYM
     definitely defines H.  */
  if (h->root.type == bfd_link_hash_new)
    return true;

  /* If H is currently a weak dynamic symbol, and if LDSYM is a strong
     dynamic symbol, LDSYM trumps the current definition of H.  */
  if ((ldsym->l_smtype & L_WEAK) == 0
      && (h->flags & XCOFF_DEF_DYNAMIC) != 0
      && (h->flags & XCOFF_DEF_REGULAR) == 0
      && (h->root.type == bfd_link_hash_defweak
	  || h->root.type == bfd_link_hash_undefweak))
    return true;

  /* If H is currently undefined, LDSYM defines it.
     However, if H has a hidden visibility, LDSYM must not
     define it.  */
  if ((h->flags & XCOFF_DEF_DYNAMIC) == 0
      && (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
      && (h->visibility != SYM_V_HIDDEN
	  && h->visibility != SYM_V_INTERNAL))
    return true;

  return false;
}

/* This function is used to add symbols from a dynamic object to the
   global symbol table.  */

static bool
xcoff_link_add_dynamic_symbols (bfd *abfd, struct bfd_link_info *info)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  const char *strings;
  bfd_byte *elsym, *elsymend;
  struct xcoff_import_file *n;
  unsigned int c;
  struct xcoff_import_file **pp;

  /* We can only handle a dynamic object if we are generating an XCOFF
     output file.  */
   if (info->output_bfd->xvec != abfd->xvec)
    {
      _bfd_error_handler
	(_("%pB: XCOFF shared object when not producing XCOFF output"),
	 abfd);
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* The symbols we use from a dynamic object are not the symbols in
     the normal symbol table, but, rather, the symbols in the export
     table.  If there is a global symbol in a dynamic object which is
     not in the export table, the loader will not be able to find it,
     so we don't want to find it either.  Also, on AIX 4.1.3, shr.o in
     libc.a has symbols in the export table which are not in the
     symbol table.  */

  /* Read in the .loader section.  FIXME: We should really use the
     o_snloader field in the a.out header, rather than grabbing the
     section by name.  */
  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    {
      _bfd_error_handler
	(_("%pB: dynamic object with no .loader section"),
	 abfd);
      bfd_set_error (bfd_error_no_symbols);
      return false;
    }

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return false;

  /* Remove the sections from this object, so that they do not get
     included in the link.  */
  bfd_section_list_clear (abfd);

  bfd_xcoff_swap_ldhdr_in (abfd, contents, &ldhdr);

  strings = (char *) contents + ldhdr.l_stoff;

  elsym = contents + bfd_xcoff_loader_symbol_offset(abfd, &ldhdr);

  elsymend = elsym + ldhdr.l_nsyms * bfd_xcoff_ldsymsz(abfd);

  for (; elsym < elsymend; elsym += bfd_xcoff_ldsymsz(abfd))
    {
      struct internal_ldsym ldsym;
      char nambuf[SYMNMLEN + 1];
      const char *name;
      struct xcoff_link_hash_entry *h;

      bfd_xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      /* We are only interested in exported symbols.  */
      if ((ldsym.l_smtype & L_EXPORT) == 0)
	continue;

      if (ldsym._l._l_l._l_zeroes == 0)
	name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  memcpy (nambuf, ldsym._l._l_name, SYMNMLEN);
	  nambuf[SYMNMLEN] = '\0';
	  name = nambuf;
	}

      /* Normally we could not call xcoff_link_hash_lookup in an add
	 symbols routine, since we might not be using an XCOFF hash
	 table.  However, we verified above that we are using an XCOFF
	 hash table.  */

      h = xcoff_link_hash_lookup (xcoff_hash_table (info), name, true,
				  true, true);
      if (h == NULL)
	return false;

      if (!xcoff_dynamic_definition_p (h, &ldsym))
	continue;

      h->flags |= XCOFF_DEF_DYNAMIC;
      h->smclas = ldsym.l_smclas;
      if (h->smclas == XMC_XO)
	{
	  /* This symbol has an absolute value.  */
	  if ((ldsym.l_smtype & L_WEAK) != 0)
	    h->root.type = bfd_link_hash_defweak;
	  else
	    h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = bfd_abs_section_ptr;
	  h->root.u.def.value = ldsym.l_value;
	}
      else
	{
	  /* Otherwise, we don't bother to actually define the symbol,
	     since we don't have a section to put it in anyhow.
	     We assume instead that an undefined XCOFF_DEF_DYNAMIC symbol
	     should be imported from the symbol's undef.abfd.  */
	  if ((ldsym.l_smtype & L_WEAK) != 0)
	    h->root.type = bfd_link_hash_undefweak;
	  else
	    h->root.type = bfd_link_hash_undefined;
	  h->root.u.undef.abfd = abfd;
	}

      /* If this symbol defines a function descriptor, then it
	 implicitly defines the function code as well.  */
      if (h->smclas == XMC_DS
	  || (h->smclas == XMC_XO && name[0] != '.'))
	h->flags |= XCOFF_DESCRIPTOR;
      if ((h->flags & XCOFF_DESCRIPTOR) != 0)
	{
	  struct xcoff_link_hash_entry *hds;

	  hds = h->descriptor;
	  if (hds == NULL)
	    {
	      char *dsnm;

	      dsnm = bfd_malloc ((bfd_size_type) strlen (name) + 2);
	      if (dsnm == NULL)
		return false;
	      dsnm[0] = '.';
	      strcpy (dsnm + 1, name);
	      hds = xcoff_link_hash_lookup (xcoff_hash_table (info), dsnm,
					    true, true, true);
	      free (dsnm);
	      if (hds == NULL)
		return false;

	      hds->descriptor = h;
	      h->descriptor = hds;
	    }

	  if (xcoff_dynamic_definition_p (hds, &ldsym))
	    {
	      hds->root.type = h->root.type;
	      hds->flags |= XCOFF_DEF_DYNAMIC;
	      if (h->smclas == XMC_XO)
		{
		  /* An absolute symbol appears to actually define code, not a
		     function descriptor.  This is how some math functions are
		     implemented on AIX 4.1.  */
		  hds->smclas = XMC_XO;
		  hds->root.u.def.section = bfd_abs_section_ptr;
		  hds->root.u.def.value = ldsym.l_value;
		}
	      else
		{
		  hds->smclas = XMC_PR;
		  hds->root.u.undef.abfd = abfd;
		  /* We do not want to add this to the undefined
		     symbol list.  */
		}
	    }
	}
    }

  free (contents);
  coff_section_data (abfd, lsec)->contents = NULL;

  /* Record this file in the import files.  */
  n = bfd_alloc (abfd, (bfd_size_type) sizeof (struct xcoff_import_file));
  if (n == NULL)
    return false;
  n->next = NULL;

  if (abfd->my_archive == NULL || bfd_is_thin_archive (abfd->my_archive))
    {
      if (!bfd_xcoff_split_import_path (abfd, bfd_get_filename (abfd),
					&n->path, &n->file))
	return false;
      n->member = "";
    }
  else
    {
      struct xcoff_archive_info *archive_info;

      archive_info = xcoff_get_archive_info (info, abfd->my_archive);
      if (!archive_info->impfile)
	{
	  if (!bfd_xcoff_split_import_path (archive_info->archive,
					    bfd_get_filename (archive_info
							      ->archive),
					    &archive_info->imppath,
					    &archive_info->impfile))
	    return false;
	}
      n->path = archive_info->imppath;
      n->file = archive_info->impfile;
      n->member = bfd_get_filename (abfd);
    }

  /* We start c at 1 because the first import file number is reserved
     for LIBPATH.  */
  for (pp = &xcoff_hash_table (info)->imports, c = 1;
       *pp != NULL;
       pp = &(*pp)->next, ++c)
    ;
  *pp = n;

  xcoff_data (abfd)->import_file_id = c;

  return true;
}

/* xcoff_link_create_extra_sections

   Takes care of creating the .loader, .gl, .ds, .debug and sections.  */

static bool
xcoff_link_create_extra_sections (bfd * abfd, struct bfd_link_info *info)
{
  bool return_value = false;

  if (info->output_bfd->xvec == abfd->xvec)
    {
      /* We need to build a .loader section, so we do it here.  This
	 won't work if we're producing an XCOFF output file with no
	 XCOFF input files.  FIXME.  */

      if (!bfd_link_relocatable (info)
	  && xcoff_hash_table (info)->loader_section == NULL)
	{
	  asection *lsec;
	  flagword flags = SEC_HAS_CONTENTS | SEC_IN_MEMORY;

	  lsec = bfd_make_section_anyway_with_flags (abfd, ".loader", flags);
	  if (lsec == NULL)
	    goto end_return;

	  xcoff_hash_table (info)->loader_section = lsec;
	}

      /* Likewise for the linkage section.  */
      if (xcoff_hash_table (info)->linkage_section == NULL)
	{
	  asection *lsec;
	  flagword flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			    | SEC_IN_MEMORY);

	  lsec = bfd_make_section_anyway_with_flags (abfd, ".gl", flags);
	  if (lsec == NULL)
	    goto end_return;

	  xcoff_hash_table (info)->linkage_section = lsec;
	  lsec->alignment_power = 2;
	}

      /* Likewise for the TOC section.  */
      if (xcoff_hash_table (info)->toc_section == NULL)
	{
	  asection *tsec;
	  flagword flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			    | SEC_IN_MEMORY);

	  tsec = bfd_make_section_anyway_with_flags (abfd, ".tc", flags);
	  if (tsec == NULL)
	    goto end_return;

	  xcoff_hash_table (info)->toc_section = tsec;
	  tsec->alignment_power = 2;
	}

      /* Likewise for the descriptor section.  */
      if (xcoff_hash_table (info)->descriptor_section == NULL)
	{
	  asection *dsec;
	  flagword flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
			    | SEC_IN_MEMORY);

	  dsec = bfd_make_section_anyway_with_flags (abfd, ".ds", flags);
	  if (dsec == NULL)
	    goto end_return;

	  xcoff_hash_table (info)->descriptor_section = dsec;
	  dsec->alignment_power = 2;
	}

      /* Likewise for the .debug section.  */
      if (xcoff_hash_table (info)->debug_section == NULL
	  && info->strip != strip_all)
	{
	  asection *dsec;
	  flagword flags = SEC_HAS_CONTENTS | SEC_IN_MEMORY;

	  dsec = bfd_make_section_anyway_with_flags (abfd, ".debug", flags);
	  if (dsec == NULL)
	    goto end_return;

	  xcoff_hash_table (info)->debug_section = dsec;
	}
    }

  return_value = true;

 end_return:

  return return_value;
}

/* Returns the index of reloc in RELOCS with the least address greater
   than or equal to ADDRESS.  The relocs are sorted by address.  */

static bfd_size_type
xcoff_find_reloc (struct internal_reloc *relocs,
		  bfd_size_type count,
		  bfd_vma address)
{
  bfd_size_type min, max, this;

  if (count < 2)
    {
      if (count == 1 && relocs[0].r_vaddr < address)
	return 1;
      else
	return 0;
    }

  min = 0;
  max = count;

  /* Do a binary search over (min,max].  */
  while (min + 1 < max)
    {
      bfd_vma raddr;

      this = (max + min) / 2;
      raddr = relocs[this].r_vaddr;
      if (raddr > address)
	max = this;
      else if (raddr < address)
	min = this;
      else
	{
	  min = this;
	  break;
	}
    }

  if (relocs[min].r_vaddr < address)
    return min + 1;

  while (min > 0
	 && relocs[min - 1].r_vaddr == address)
    --min;

  return min;
}

/* Return true if the symbol has to be added to the linker hash
   table.  */
static bool
xcoff_link_add_symbols_to_hash_table (struct internal_syment sym,
				      union internal_auxent aux)
{
  /* External symbols must be added.  */
  if (EXTERN_SYM_P (sym.n_sclass))
    return true;

  /* Hidden TLS symbols must be added to verify TLS relocations
     in xcoff_reloc_type_tls.  */
  if (sym.n_sclass == C_HIDEXT
      && ((aux.x_csect.x_smclas == XMC_TL
	   || aux.x_csect.x_smclas == XMC_UL)))
    return true;

  return false;
}

/* Add all the symbols from an object file to the hash table.

   XCOFF is a weird format.  A normal XCOFF .o files will have three
   COFF sections--.text, .data, and .bss--but each COFF section will
   contain many csects.  These csects are described in the symbol
   table.  From the linker's point of view, each csect must be
   considered a section in its own right.  For example, a TOC entry is
   handled as a small XMC_TC csect.  The linker must be able to merge
   different TOC entries together, which means that it must be able to
   extract the XMC_TC csects from the .data section of the input .o
   file.

   From the point of view of our linker, this is, of course, a hideous
   nightmare.  We cope by actually creating sections for each csect,
   and discarding the original sections.  We then have to handle the
   relocation entries carefully, since the only way to tell which
   csect they belong to is to examine the address.  */

static bool
xcoff_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  unsigned int n_tmask;
  unsigned int n_btshft;
  bool default_copy;
  bfd_size_type symcount;
  struct xcoff_link_hash_entry **sym_hash;
  asection **csect_cache;
  unsigned int *lineno_counts;
  bfd_size_type linesz;
  asection *o;
  asection *last_real;
  bool keep_syms;
  asection *csect;
  unsigned int csect_index;
  asection *first_csect;
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;
  struct reloc_info_struct
  {
    struct internal_reloc *relocs;
    asection **csects;
    bfd_byte *linenos;
  } *reloc_info = NULL;
  bfd_size_type amt;
  unsigned short visibility;

  keep_syms = obj_coff_keep_syms (abfd);

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link)
    {
      if (! xcoff_link_add_dynamic_symbols (abfd, info))
	return false;
    }

  /* Create the loader, toc, gl, ds and debug sections, if needed.  */
  if (! xcoff_link_create_extra_sections (abfd, info))
    goto error_return;

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link)
    return true;

  n_tmask = coff_data (abfd)->local_n_tmask;
  n_btshft = coff_data (abfd)->local_n_btshft;

  /* Define macros so that ISFCN, et. al., macros work correctly.  */
#define N_TMASK n_tmask
#define N_BTSHFT n_btshft

  if (info->keep_memory)
    default_copy = false;
  else
    default_copy = true;

  symcount = obj_raw_syment_count (abfd);

  /* We keep a list of the linker hash table entries that correspond
     to each external symbol.  */
  amt = symcount * sizeof (struct xcoff_link_hash_entry *);
  sym_hash = bfd_zalloc (abfd, amt);
  if (sym_hash == NULL && symcount != 0)
    goto error_return;
  coff_data (abfd)->sym_hashes = (struct coff_link_hash_entry **) sym_hash;

  /* Because of the weird stuff we are doing with XCOFF csects, we can
     not easily determine which section a symbol is in, so we store
     the information in the tdata for the input file.  */
  amt = symcount * sizeof (asection *);
  csect_cache = bfd_zalloc (abfd, amt);
  if (csect_cache == NULL && symcount != 0)
    goto error_return;
  xcoff_data (abfd)->csects = csect_cache;

  /* We garbage-collect line-number information on a symbol-by-symbol
     basis, so we need to have quick access to the number of entries
     per symbol.  */
  amt = symcount * sizeof (unsigned int);
  lineno_counts = bfd_zalloc (abfd, amt);
  if (lineno_counts == NULL && symcount != 0)
    goto error_return;
  xcoff_data (abfd)->lineno_counts = lineno_counts;

  /* While splitting sections into csects, we need to assign the
     relocs correctly.  The relocs and the csects must both be in
     order by VMA within a given section, so we handle this by
     scanning along the relocs as we process the csects.  We index
     into reloc_info using the section target_index.  */
  amt = abfd->section_count + 1;
  amt *= sizeof (struct reloc_info_struct);
  reloc_info = bfd_zmalloc (amt);
  if (reloc_info == NULL)
    goto error_return;

  /* Read in the relocs and line numbers for each section.  */
  linesz = bfd_coff_linesz (abfd);
  last_real = NULL;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      last_real = o;

      if ((o->flags & SEC_RELOC) != 0)
	{
	  reloc_info[o->target_index].relocs =
	    xcoff_read_internal_relocs (abfd, o, true, NULL, false, NULL);
	  amt = o->reloc_count;
	  amt *= sizeof (asection *);
	  reloc_info[o->target_index].csects = bfd_zmalloc (amt);
	  if (reloc_info[o->target_index].csects == NULL)
	    goto error_return;
	}

      if ((info->strip == strip_none || info->strip == strip_some)
	  && o->lineno_count > 0)
	{
	  bfd_byte *linenos;

	  if (bfd_seek (abfd, o->line_filepos, SEEK_SET) != 0)
	    goto error_return;
	  if (_bfd_mul_overflow (linesz, o->lineno_count, &amt))
	    {
	      bfd_set_error (bfd_error_file_too_big);
	      goto error_return;
	    }
	  linenos = _bfd_malloc_and_read (abfd, amt, amt);
	  if (linenos == NULL)
	    goto error_return;
	  reloc_info[o->target_index].linenos = linenos;
	}
    }

  /* Don't let the linker relocation routines discard the symbols.  */
  obj_coff_keep_syms (abfd) = true;

  csect = NULL;
  csect_index = 0;
  first_csect = NULL;

  symesz = bfd_coff_symesz (abfd);
  BFD_ASSERT (symesz == bfd_coff_auxesz (abfd));
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + symcount * symesz;

  while (esym < esym_end)
    {
      struct internal_syment sym;
      union internal_auxent aux;
      const char *name;
      char buf[SYMNMLEN + 1];
      int smtyp;
      asection *section;
      bfd_vma value;
      struct xcoff_link_hash_entry *set_toc;

      bfd_coff_swap_sym_in (abfd, (void *) esym, (void *) &sym);

      /* In this pass we are only interested in symbols with csect
	 information.  */
      if (!CSECT_SYM_P (sym.n_sclass))
	{
	  /* Set csect_cache,
	     Normally csect is a .pr, .rw  etc. created in the loop
	     If C_FILE or first time, handle special

	     Advance esym, sym_hash, csect_hash ptrs.  */
	  if (sym.n_sclass == C_FILE || sym.n_sclass == C_DWARF)
	    csect = NULL;
	  if (csect != NULL)
	    *csect_cache = csect;
	  else if (first_csect == NULL
		   || sym.n_sclass == C_FILE || sym.n_sclass == C_DWARF)
	    *csect_cache = coff_section_from_bfd_index (abfd, sym.n_scnum);
	  else
	    *csect_cache = NULL;
	  esym += (sym.n_numaux + 1) * symesz;
	  sym_hash += sym.n_numaux + 1;
	  csect_cache += sym.n_numaux + 1;
	  lineno_counts += sym.n_numaux + 1;

	  continue;
	}

      name = _bfd_coff_internal_syment_name (abfd, &sym, buf);

      if (name == NULL)
	goto error_return;

      /* If this symbol has line number information attached to it,
	 and we're not stripping it, count the number of entries and
	 add them to the count for this csect.  In the final link pass
	 we are going to attach line number information by symbol,
	 rather than by section, in order to more easily handle
	 garbage collection.  */
      if ((info->strip == strip_none || info->strip == strip_some)
	  && sym.n_numaux > 1
	  && csect != NULL
	  && ISFCN (sym.n_type))
	{
	  union internal_auxent auxlin;

	  bfd_coff_swap_aux_in (abfd, (void *) (esym + symesz),
				sym.n_type, sym.n_sclass,
				0, sym.n_numaux, (void *) &auxlin);

	  if (auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr != 0)
	    {
	      asection *enclosing;
	      bfd_signed_vma linoff;

	      enclosing = xcoff_section_data (abfd, csect)->enclosing;
	      if (enclosing == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: `%s' has line numbers but no enclosing section"),
		     abfd, name);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      linoff = (auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr
			- enclosing->line_filepos);
	      /* Explicit cast to bfd_signed_vma for compiler.  */
	      if (linoff < (bfd_signed_vma) (enclosing->lineno_count * linesz))
		{
		  struct internal_lineno lin;
		  bfd_byte *linpstart;

		  linpstart = (reloc_info[enclosing->target_index].linenos
			       + linoff);
		  bfd_coff_swap_lineno_in (abfd, (void *) linpstart, (void *) &lin);
		  if (lin.l_lnno == 0
		      && ((bfd_size_type) lin.l_addr.l_symndx
			  == ((esym
			       - (bfd_byte *) obj_coff_external_syms (abfd))
			      / symesz)))
		    {
		      bfd_byte *linpend, *linp;

		      linpend = (reloc_info[enclosing->target_index].linenos
				 + enclosing->lineno_count * linesz);
		      for (linp = linpstart + linesz;
			   linp < linpend;
			   linp += linesz)
			{
			  bfd_coff_swap_lineno_in (abfd, (void *) linp,
						   (void *) &lin);
			  if (lin.l_lnno == 0)
			    break;
			}
		      *lineno_counts = (linp - linpstart) / linesz;
		      /* The setting of line_filepos will only be
			 useful if all the line number entries for a
			 csect are contiguous; this only matters for
			 error reporting.  */
		      if (csect->line_filepos == 0)
			csect->line_filepos =
			  auxlin.x_sym.x_fcnary.x_fcn.x_lnnoptr;
		    }
		}
	    }
	}

      /* Record visibility.  */
      visibility = sym.n_type & SYM_V_MASK;

      /* Pick up the csect auxiliary information.  */
      if (sym.n_numaux == 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: class %d symbol `%s' has no aux entries"),
	     abfd, sym.n_sclass, name);
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;
	}

      bfd_coff_swap_aux_in (abfd,
			    (void *) (esym + symesz * sym.n_numaux),
			    sym.n_type, sym.n_sclass,
			    sym.n_numaux - 1, sym.n_numaux,
			    (void *) &aux);

      smtyp = SMTYP_SMTYP (aux.x_csect.x_smtyp);

      section = NULL;
      value = 0;
      set_toc = NULL;

      switch (smtyp)
	{
	default:
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: symbol `%s' has unrecognized csect type %d"),
	     abfd, name, smtyp);
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;

	case XTY_ER:
	  /* This is an external reference.  */
	  if (sym.n_sclass == C_HIDEXT
	      || sym.n_scnum != N_UNDEF
	      || aux.x_csect.x_scnlen.u64 != 0)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: bad XTY_ER symbol `%s': class %d scnum %d "
		   "scnlen %" PRId64),
		 abfd, name, sym.n_sclass, sym.n_scnum,
		 aux.x_csect.x_scnlen.u64);
	      bfd_set_error (bfd_error_bad_value);
	      goto error_return;
	    }

	  /* An XMC_XO external reference is actually a reference to
	     an absolute location.  */
	  if (aux.x_csect.x_smclas != XMC_XO)
	    section = bfd_und_section_ptr;
	  else
	    {
	      section = bfd_abs_section_ptr;
	      value = sym.n_value;
	    }
	  break;

	case XTY_SD:
	  csect = NULL;
	  csect_index = -(unsigned) 1;

	  /* When we see a TOC anchor, we record the TOC value.  */
	  if (aux.x_csect.x_smclas == XMC_TC0)
	    {
	      if (sym.n_sclass != C_HIDEXT
		  || aux.x_csect.x_scnlen.u64 != 0)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: XMC_TC0 symbol `%s' is class %d scnlen %" PRIu64),
		     abfd, name, sym.n_sclass, aux.x_csect.x_scnlen.u64);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      xcoff_data (abfd)->toc = sym.n_value;
	    }

	  /* We must merge TOC entries for the same symbol.  We can
	     merge two TOC entries if they are both C_HIDEXT, they
	     both have the same name, they are both 4 or 8 bytes long, and
	     they both have a relocation table entry for an external
	     symbol with the same name.  Unfortunately, this means
	     that we must look through the relocations.  Ick.

	     Logic for 32 bit vs 64 bit.
	     32 bit has a csect length of 4 for TOC
	     64 bit has a csect length of 8 for TOC

	     An exception is made for TOC entries with a R_TLSML
	     relocation.  This relocation is made for the loader.
	     We must check that the referenced symbol is the TOC entry
	     itself.

	     The conditions to get past the if-check are not that bad.
	     They are what is used to create the TOC csects in the first
	     place.  */
	  if (aux.x_csect.x_smclas == XMC_TC
	      && sym.n_sclass == C_HIDEXT
	      && info->output_bfd->xvec == abfd->xvec
	      && ((bfd_xcoff_is_xcoff32 (abfd)
		   && aux.x_csect.x_scnlen.u64 == 4)
		  || (bfd_xcoff_is_xcoff64 (abfd)
		      && aux.x_csect.x_scnlen.u64 == 8)))
	    {
	      asection *enclosing;
	      struct internal_reloc *relocs;
	      bfd_size_type relindx;
	      struct internal_reloc *rel;

	      enclosing = coff_section_from_bfd_index (abfd, sym.n_scnum);
	      if (enclosing == NULL)
		goto error_return;

	      relocs = reloc_info[enclosing->target_index].relocs;
	      amt = enclosing->reloc_count;
	      relindx = xcoff_find_reloc (relocs, amt, sym.n_value);
	      rel = relocs + relindx;

	      /* 32 bit R_POS r_size is 31
		 64 bit R_POS r_size is 63  */
	      if (relindx < enclosing->reloc_count
		  && rel->r_vaddr == (bfd_vma) sym.n_value
		  && (rel->r_type == R_POS ||
		      rel->r_type == R_TLSML)
		  && ((bfd_xcoff_is_xcoff32 (abfd)
		       && rel->r_size == 31)
		      || (bfd_xcoff_is_xcoff64 (abfd)
			  && rel->r_size == 63)))
		{
		  bfd_byte *erelsym;

		  struct internal_syment relsym;

		  erelsym = ((bfd_byte *) obj_coff_external_syms (abfd)
			     + rel->r_symndx * symesz);
		  bfd_coff_swap_sym_in (abfd, (void *) erelsym, (void *) &relsym);
		  if (EXTERN_SYM_P (relsym.n_sclass))
		    {
		      const char *relname;
		      char relbuf[SYMNMLEN + 1];
		      bool copy;
		      struct xcoff_link_hash_entry *h;

		      /* At this point we know that the TOC entry is
			 for an externally visible symbol.  */
		      relname = _bfd_coff_internal_syment_name (abfd, &relsym,
								relbuf);
		      if (relname == NULL)
			goto error_return;

		      /* We only merge TOC entries if the TC name is
			 the same as the symbol name.  This handles
			 the normal case, but not common cases like
			 SYM.P4 which gcc generates to store SYM + 4
			 in the TOC.  FIXME.  */
		      if (strcmp (name, relname) == 0)
			{
			  copy = (! info->keep_memory
				  || relsym._n._n_n._n_zeroes != 0
				  || relsym._n._n_n._n_offset == 0);
			  h = xcoff_link_hash_lookup (xcoff_hash_table (info),
						      relname, true, copy,
						      false);
			  if (h == NULL)
			    goto error_return;

			  /* At this point h->root.type could be
			     bfd_link_hash_new.  That should be OK,
			     since we know for sure that we will come
			     across this symbol as we step through the
			     file.  */

			  /* We store h in *sym_hash for the
			     convenience of the relocate_section
			     function.  */
			  *sym_hash = h;

			  if (h->toc_section != NULL)
			    {
			      asection **rel_csects;

			      /* We already have a TOC entry for this
				 symbol, so we can just ignore this
				 one.  */
			      rel_csects =
				reloc_info[enclosing->target_index].csects;
			      rel_csects[relindx] = bfd_und_section_ptr;
			      break;
			    }

			  /* We are about to create a TOC entry for
			     this symbol.  */
			  set_toc = h;
			}
		    }
		  else if (rel->r_type == R_TLSML)
		    {
			csect_index = ((esym
					- (bfd_byte *) obj_coff_external_syms (abfd))
				       / symesz);
			if (((unsigned long) rel->r_symndx) != csect_index)
			  {
			    _bfd_error_handler
			      /* xgettext:c-format */
			      (_("%pB: TOC entry `%s' has a R_TLSML"
				 "relocation not targeting itself"),
			       abfd, name);
			    bfd_set_error (bfd_error_bad_value);
			    goto error_return;
			  }
		    }
		}
	    }

	  {
	    asection *enclosing;

	    /* We need to create a new section.  We get the name from
	       the csect storage mapping class, so that the linker can
	       accumulate similar csects together.  */

	    csect = bfd_xcoff_create_csect_from_smclas(abfd, &aux, name);
	    if (NULL == csect)
	      goto error_return;

	    /* The enclosing section is the main section : .data, .text
	       or .bss that the csect is coming from.  */
	    enclosing = coff_section_from_bfd_index (abfd, sym.n_scnum);
	    if (enclosing == NULL)
	      goto error_return;

	    if (! bfd_is_abs_section (enclosing)
		&& ((bfd_vma) sym.n_value < enclosing->vma
		    || (sym.n_value + aux.x_csect.x_scnlen.u64
			> enclosing->vma + enclosing->size)))
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: csect `%s' not in enclosing section"),
		   abfd, name);
		bfd_set_error (bfd_error_bad_value);
		goto error_return;
	      }
	    csect->vma = sym.n_value;
	    csect->filepos = (enclosing->filepos
			      + sym.n_value
			      - enclosing->vma);
	    csect->size = aux.x_csect.x_scnlen.u64;
	    csect->rawsize = aux.x_csect.x_scnlen.u64;
	    csect->flags |= SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS;
	    csect->alignment_power = SMTYP_ALIGN (aux.x_csect.x_smtyp);

	    /* Record the enclosing section in the tdata for this new
	       section.  */
	    amt = sizeof (struct coff_section_tdata);
	    csect->used_by_bfd = bfd_zalloc (abfd, amt);
	    if (csect->used_by_bfd == NULL)
	      goto error_return;
	    amt = sizeof (struct xcoff_section_tdata);
	    coff_section_data (abfd, csect)->tdata = bfd_zalloc (abfd, amt);
	    if (coff_section_data (abfd, csect)->tdata == NULL)
	      goto error_return;
	    xcoff_section_data (abfd, csect)->enclosing = enclosing;
	    xcoff_section_data (abfd, csect)->lineno_count =
	      enclosing->lineno_count;

	    if (enclosing->owner == abfd)
	      {
		struct internal_reloc *relocs;
		bfd_size_type relindx;
		struct internal_reloc *rel;
		asection **rel_csect;

		relocs = reloc_info[enclosing->target_index].relocs;
		amt = enclosing->reloc_count;
		relindx = xcoff_find_reloc (relocs, amt, csect->vma);

		rel = relocs + relindx;
		rel_csect = (reloc_info[enclosing->target_index].csects
			     + relindx);

		csect->rel_filepos = (enclosing->rel_filepos
				      + relindx * bfd_coff_relsz (abfd));
		while (relindx < enclosing->reloc_count
		       && *rel_csect == NULL
		       && rel->r_vaddr < csect->vma + csect->size)
		  {

		    *rel_csect = csect;
		    csect->flags |= SEC_RELOC;
		    ++csect->reloc_count;
		    ++relindx;
		    ++rel;
		    ++rel_csect;
		  }
	      }

	    /* There are a number of other fields and section flags
	       which we do not bother to set.  */

	    csect_index = ((esym
			    - (bfd_byte *) obj_coff_external_syms (abfd))
			   / symesz);

	    xcoff_section_data (abfd, csect)->first_symndx = csect_index;

	    if (first_csect == NULL)
	      first_csect = csect;

	    /* If this symbol must be added to the linker hash table,
	       we treat it as starting at the beginning of the newly
	       created section.  */
	    if (xcoff_link_add_symbols_to_hash_table (sym, aux))
	      {
		section = csect;
		value = 0;
	      }

	    /* If this is a TOC section for a symbol, record it.  */
	    if (set_toc != NULL)
	      set_toc->toc_section = csect;
	  }
	  break;

	case XTY_LD:
	  /* This is a label definition.  The x_scnlen field is the
	     symbol index of the csect.  Usually the XTY_LD symbol will
	     follow its appropriate XTY_SD symbol.  The .set pseudo op can
	     cause the XTY_LD to not follow the XTY_SD symbol. */
	  {
	    bool bad;

	    bad = false;
	    if (aux.x_csect.x_scnlen.u64
		>= (size_t) (esym - (bfd_byte *) obj_coff_external_syms (abfd)))
	      bad = true;
	    if (! bad)
	      {
		section = xcoff_data (abfd)->csects[aux.x_csect.x_scnlen.u64];
		if (section == NULL
		    || (section->flags & SEC_HAS_CONTENTS) == 0)
		  bad = true;
	      }
	    if (bad)
	      {
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("%pB: misplaced XTY_LD `%s'"),
		   abfd, name);
		bfd_set_error (bfd_error_bad_value);
		goto error_return;
	      }
	    csect = section;
	    value = sym.n_value - csect->vma;
	  }
	  break;

	case XTY_CM:
	  /* This is an unitialized csect.  We could base the name on
	     the storage mapping class, but we don't bother except for
	     an XMC_TD symbol.  If this csect is externally visible,
	     it is a common symbol.  We put XMC_TD symbols in sections
	     named .tocbss, and rely on the linker script to put that
	     in the TOC area.  */

	  if (aux.x_csect.x_smclas == XMC_TD)
	    {
	      /* The linker script puts the .td section in the data
		 section after the .tc section.  */
	      csect = bfd_make_section_anyway_with_flags (abfd, ".td",
							  SEC_ALLOC);
	    }
	  else if (aux.x_csect.x_smclas == XMC_UL)
	    {
	      /* This is a thread-local unitialized csect.  */
	      csect = bfd_make_section_anyway_with_flags (abfd, ".tbss",
							  SEC_ALLOC | SEC_THREAD_LOCAL);
	    }
	  else
	    csect = bfd_make_section_anyway_with_flags (abfd, ".bss",
							SEC_ALLOC);

	  if (csect == NULL)
	    goto error_return;
	  csect->vma = sym.n_value;
	  csect->size = aux.x_csect.x_scnlen.u64;
	  csect->alignment_power = SMTYP_ALIGN (aux.x_csect.x_smtyp);
	  /* There are a number of other fields and section flags
	     which we do not bother to set.  */

	  csect_index = ((esym
			  - (bfd_byte *) obj_coff_external_syms (abfd))
			 / symesz);

	  amt = sizeof (struct coff_section_tdata);
	  csect->used_by_bfd = bfd_zalloc (abfd, amt);
	  if (csect->used_by_bfd == NULL)
	    goto error_return;
	  amt = sizeof (struct xcoff_section_tdata);
	  coff_section_data (abfd, csect)->tdata = bfd_zalloc (abfd, amt);
	  if (coff_section_data (abfd, csect)->tdata == NULL)
	    goto error_return;
	  xcoff_section_data (abfd, csect)->first_symndx = csect_index;

	  if (first_csect == NULL)
	    first_csect = csect;

	  if (xcoff_link_add_symbols_to_hash_table (sym, aux))
	    {
	      csect->flags |= SEC_IS_COMMON;
	      csect->size = 0;
	      section = csect;
	      value = aux.x_csect.x_scnlen.u64;
	    }

	  break;
	}

      /* Check for magic symbol names.  */
      if ((smtyp == XTY_SD || smtyp == XTY_CM)
	  && aux.x_csect.x_smclas != XMC_TC
	  && aux.x_csect.x_smclas != XMC_TD)
	{
	  int i = -1;

	  if (name[0] == '_')
	    {
	      if (strcmp (name, "_text") == 0)
		i = XCOFF_SPECIAL_SECTION_TEXT;
	      else if (strcmp (name, "_etext") == 0)
		i = XCOFF_SPECIAL_SECTION_ETEXT;
	      else if (strcmp (name, "_data") == 0)
		i = XCOFF_SPECIAL_SECTION_DATA;
	      else if (strcmp (name, "_edata") == 0)
		i = XCOFF_SPECIAL_SECTION_EDATA;
	      else if (strcmp (name, "_end") == 0)
		i = XCOFF_SPECIAL_SECTION_END;
	    }
	  else if (name[0] == 'e' && strcmp (name, "end") == 0)
	    i = XCOFF_SPECIAL_SECTION_END2;

	  if (i != -1)
	    xcoff_hash_table (info)->special_sections[i] = csect;
	}

      /* Now we have enough information to add the symbol to the
	 linker hash table.  */

      if (xcoff_link_add_symbols_to_hash_table (sym, aux))
	{
	  bool copy, ok;
	  flagword flags;

	  BFD_ASSERT (section != NULL);

	  /* We must copy the name into memory if we got it from the
	     syment itself, rather than the string table.  */
	  copy = default_copy;
	  if (sym._n._n_n._n_zeroes != 0
	      || sym._n._n_n._n_offset == 0)
	    copy = true;

	  /* Ignore global linkage code when linking statically.  */
	  if (info->static_link
	      && (smtyp == XTY_SD || smtyp == XTY_LD)
	      && aux.x_csect.x_smclas == XMC_GL)
	    {
	      section = bfd_und_section_ptr;
	      value = 0;
	    }

	  /* The AIX linker appears to only detect multiple symbol
	     definitions when there is a reference to the symbol.  If
	     a symbol is defined multiple times, and the only
	     references are from the same object file, the AIX linker
	     appears to permit it.  It does not merge the different
	     definitions, but handles them independently.  On the
	     other hand, if there is a reference, the linker reports
	     an error.

	     This matters because the AIX <net/net_globals.h> header
	     file actually defines an initialized array, so we have to
	     actually permit that to work.

	     Just to make matters even more confusing, the AIX linker
	     appears to permit multiple symbol definitions whenever
	     the second definition is in an archive rather than an
	     object file.  This may be a consequence of the manner in
	     which it handles archives: I think it may load the entire
	     archive in as separate csects, and then let garbage
	     collection discard symbols.

	     We also have to handle the case of statically linking a
	     shared object, which will cause symbol redefinitions,
	     although this is an easier case to detect.  */
	  else if (info->output_bfd->xvec == abfd->xvec)
	    {
	      if (! bfd_is_und_section (section))
		*sym_hash = xcoff_link_hash_lookup (xcoff_hash_table (info),
						    name, true, copy, false);
	      else
		/* Make a copy of the symbol name to prevent problems with
		   merging symbols.  */
		*sym_hash = ((struct xcoff_link_hash_entry *)
			     bfd_wrapped_link_hash_lookup (abfd, info, name,
							   true, true, false));

	      if (*sym_hash == NULL)
		goto error_return;
	      if (((*sym_hash)->root.type == bfd_link_hash_defined
		   || (*sym_hash)->root.type == bfd_link_hash_defweak)
		  && ! bfd_is_und_section (section)
		  && ! bfd_is_com_section (section))
		{
		  /* This is a second definition of a defined symbol.  */
		  if (((*sym_hash)->flags & XCOFF_DEF_REGULAR) == 0
		      && ((*sym_hash)->flags & XCOFF_DEF_DYNAMIC) != 0)
		    {
		      /* The existing symbol is from a shared library.
			 Replace it.  */
		      (*sym_hash)->root.type = bfd_link_hash_undefined;
		      (*sym_hash)->root.u.undef.abfd =
			(*sym_hash)->root.u.def.section->owner;
		    }
		  else if (abfd->my_archive != NULL)
		    {
		      /* This is a redefinition in an object contained
			 in an archive.  Just ignore it.  See the
			 comment above.  */
		      section = bfd_und_section_ptr;
		      value = 0;
		    }
		  else if (sym.n_sclass == C_AIX_WEAKEXT
			   || (*sym_hash)->root.type == bfd_link_hash_defweak)
		    {
		      /* At least one of the definitions is weak.
			 Allow the normal rules to take effect.  */
		    }
		  else if ((*sym_hash)->root.u.undef.next != NULL
			   || info->hash->undefs_tail == &(*sym_hash)->root)
		    {
		      /* This symbol has been referenced.  In this
			 case, we just continue and permit the
			 multiple definition error.  See the comment
			 above about the behaviour of the AIX linker.  */
		    }
		  else if ((*sym_hash)->smclas == aux.x_csect.x_smclas)
		    {
		      /* The symbols are both csects of the same
			 class.  There is at least a chance that this
			 is a semi-legitimate redefinition.  */
		      section = bfd_und_section_ptr;
		      value = 0;
		      (*sym_hash)->flags |= XCOFF_MULTIPLY_DEFINED;
		    }
		}
	      else if (((*sym_hash)->flags & XCOFF_MULTIPLY_DEFINED) != 0
		       && (*sym_hash)->root.type == bfd_link_hash_defined
		       && (bfd_is_und_section (section)
			   || bfd_is_com_section (section)))
		{
		  /* This is a reference to a multiply defined symbol.
		     Report the error now.  See the comment above
		     about the behaviour of the AIX linker.  We could
		     also do this with warning symbols, but I'm not
		     sure the XCOFF linker is wholly prepared to
		     handle them, and that would only be a warning,
		     not an error.  */
		  (*info->callbacks->multiple_definition) (info,
							   &(*sym_hash)->root,
							   NULL, NULL,
							   (bfd_vma) 0);
		  /* Try not to give this error too many times.  */
		  (*sym_hash)->flags &= ~XCOFF_MULTIPLY_DEFINED;
		}


	      /* If the symbol is hidden or internal, completely undo
		 any dynamic link state.  */
	      if ((*sym_hash)->flags & XCOFF_DEF_DYNAMIC
		  && (visibility == SYM_V_HIDDEN
		      || visibility == SYM_V_INTERNAL))
		  (*sym_hash)->flags &= ~XCOFF_DEF_DYNAMIC;
	      else
		{
		  /* Keep the most constraining visibility.  */
		  unsigned short hvis = (*sym_hash)->visibility;
		  if (visibility && ( !hvis || visibility < hvis))
		    (*sym_hash)->visibility = visibility;
		}

	    }

	  /* _bfd_generic_link_add_one_symbol may call the linker to
	     generate an error message, and the linker may try to read
	     the symbol table to give a good error.  Right now, the
	     line numbers are in an inconsistent state, since they are
	     counted both in the real sections and in the new csects.
	     We need to leave the count in the real sections so that
	     the linker can report the line number of the error
	     correctly, so temporarily clobber the link to the csects
	     so that the linker will not try to read the line numbers
	     a second time from the csects.  */
	  BFD_ASSERT (last_real->next == first_csect);
	  last_real->next = NULL;
	  flags = (sym.n_sclass == C_EXT ? BSF_GLOBAL : BSF_WEAK);
	  ok = (_bfd_generic_link_add_one_symbol
		(info, abfd, name, flags, section, value, NULL, copy, true,
		 (struct bfd_link_hash_entry **) sym_hash));
	  last_real->next = first_csect;
	  if (!ok)
	    goto error_return;

	  if (smtyp == XTY_CM)
	    {
	      if ((*sym_hash)->root.type != bfd_link_hash_common
		  || (*sym_hash)->root.u.c.p->section != csect)
		/* We don't need the common csect we just created.  */
		csect->size = 0;
	      else
		(*sym_hash)->root.u.c.p->alignment_power
		  = csect->alignment_power;
	    }

	  if (info->output_bfd->xvec == abfd->xvec)
	    {
	      int flag;

	      if (smtyp == XTY_ER
		  || smtyp == XTY_CM
		  || section == bfd_und_section_ptr)
		flag = XCOFF_REF_REGULAR;
	      else
		flag = XCOFF_DEF_REGULAR;
	      (*sym_hash)->flags |= flag;

	      if ((*sym_hash)->smclas == XMC_UA
		  || flag == XCOFF_DEF_REGULAR)
		(*sym_hash)->smclas = aux.x_csect.x_smclas;
	    }
	}

      if (smtyp == XTY_ER)
	*csect_cache = section;
      else
	{
	  *csect_cache = csect;
	  if (csect != NULL)
	    xcoff_section_data (abfd, csect)->last_symndx
	      = (esym - (bfd_byte *) obj_coff_external_syms (abfd)) / symesz;
	}

      esym += (sym.n_numaux + 1) * symesz;
      sym_hash += sym.n_numaux + 1;
      csect_cache += sym.n_numaux + 1;
      lineno_counts += sym.n_numaux + 1;
    }

  BFD_ASSERT (last_real == NULL || last_real->next == first_csect);

  /* Make sure that we have seen all the relocs.  */
  for (o = abfd->sections; o != first_csect; o = o->next)
    {
      /* Debugging sections have no csects.  */
      if (bfd_section_flags (o) & SEC_DEBUGGING)
	continue;

      /* Reset the section size and the line number count, since the
	 data is now attached to the csects.  Don't reset the size of
	 the .debug section, since we need to read it below in
	 bfd_xcoff_size_dynamic_sections.  */
      if (strcmp (bfd_section_name (o), ".debug") != 0)
	o->size = 0;
      o->lineno_count = 0;

      if ((o->flags & SEC_RELOC) != 0)
	{
	  bfd_size_type i;
	  struct internal_reloc *rel;
	  asection **rel_csect;

	  rel = reloc_info[o->target_index].relocs;
	  rel_csect = reloc_info[o->target_index].csects;

	  for (i = 0; i < o->reloc_count; i++, rel++, rel_csect++)
	    {
	      if (*rel_csect == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: reloc %s:%" PRId64 " not in csect"),
		     abfd, o->name, (int64_t) i);
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}

	      /* We identify all function symbols that are the target
		 of a relocation, so that we can create glue code for
		 functions imported from dynamic objects.  */
	      if (info->output_bfd->xvec == abfd->xvec
		  && *rel_csect != bfd_und_section_ptr
		  && obj_xcoff_sym_hashes (abfd)[rel->r_symndx] != NULL)
		{
		  struct xcoff_link_hash_entry *h;

		  h = obj_xcoff_sym_hashes (abfd)[rel->r_symndx];
		  /* If the symbol name starts with a period, it is
		     the code of a function.  If the symbol is
		     currently undefined, then add an undefined symbol
		     for the function descriptor.  This should do no
		     harm, because any regular object that defines the
		     function should also define the function
		     descriptor.  It helps, because it means that we
		     will identify the function descriptor with a
		     dynamic object if a dynamic object defines it.  */
		  if (h->root.root.string[0] == '.'
		      && h->descriptor == NULL)
		    {
		      struct xcoff_link_hash_entry *hds;
		      struct bfd_link_hash_entry *bh;

		      hds = xcoff_link_hash_lookup (xcoff_hash_table (info),
						    h->root.root.string + 1,
						    true, false, true);
		      if (hds == NULL)
			goto error_return;
		      if (hds->root.type == bfd_link_hash_new)
			{
			  bh = &hds->root;
			  if (! (_bfd_generic_link_add_one_symbol
				 (info, abfd, hds->root.root.string,
				  (flagword) 0, bfd_und_section_ptr,
				  (bfd_vma) 0, NULL, false,
				  true, &bh)))
			    goto error_return;
			  hds = (struct xcoff_link_hash_entry *) bh;
			}
		      hds->flags |= XCOFF_DESCRIPTOR;
		      BFD_ASSERT ((h->flags & XCOFF_DESCRIPTOR) == 0);
		      hds->descriptor = h;
		      h->descriptor = hds;
		    }
		  if (h->root.root.string[0] == '.')
		    h->flags |= XCOFF_CALLED;
		}
	    }

	  free (reloc_info[o->target_index].csects);
	  reloc_info[o->target_index].csects = NULL;

	  /* Reset SEC_RELOC and the reloc_count, since the reloc
	     information is now attached to the csects.  */
	  o->flags &=~ SEC_RELOC;
	  o->reloc_count = 0;

	  /* If we are not keeping memory, free the reloc information.  */
	  if (! info->keep_memory
	      && coff_section_data (abfd, o) != NULL)
	    {
	      free (coff_section_data (abfd, o)->relocs);
	      coff_section_data (abfd, o)->relocs = NULL;
	    }
	}

      /* Free up the line numbers.  FIXME: We could cache these
	 somewhere for the final link, to avoid reading them again.  */
      free (reloc_info[o->target_index].linenos);
      reloc_info[o->target_index].linenos = NULL;
    }

  free (reloc_info);

  obj_coff_keep_syms (abfd) = keep_syms;

  return true;

 error_return:
  if (reloc_info != NULL)
    {
      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  free (reloc_info[o->target_index].csects);
	  free (reloc_info[o->target_index].linenos);
	}
      free (reloc_info);
    }
  obj_coff_keep_syms (abfd) = keep_syms;
  return false;
}

#undef N_TMASK
#undef N_BTSHFT

/* Add symbols from an XCOFF object file.  */

static bool
xcoff_link_add_object_symbols (bfd *abfd, struct bfd_link_info *info)
{
  if (! _bfd_coff_get_external_symbols (abfd))
    return false;
  if (! xcoff_link_add_symbols (abfd, info))
    return false;
  if (! info->keep_memory)
    {
      if (! _bfd_coff_free_symbols (abfd))
	return false;
    }
  return true;
}

/* Look through the loader symbols to see if this dynamic object
   should be included in the link.  The native linker uses the loader
   symbols, not the normal symbol table, so we do too.  */

static bool
xcoff_link_check_dynamic_ar_symbols (bfd *abfd,
				     struct bfd_link_info *info,
				     bool *pneeded,
				     bfd **subsbfd)
{
  asection *lsec;
  bfd_byte *contents;
  struct internal_ldhdr ldhdr;
  const char *strings;
  bfd_byte *elsym, *elsymend;

  *pneeded = false;

  lsec = bfd_get_section_by_name (abfd, ".loader");
  if (lsec == NULL || (lsec->flags & SEC_HAS_CONTENTS) == 0)
    /* There are no symbols, so don't try to include it.  */
    return true;

  contents = xcoff_get_section_contents (abfd, lsec);
  if (!contents)
    return false;

  bfd_xcoff_swap_ldhdr_in (abfd, contents, &ldhdr);

  strings = (char *) contents + ldhdr.l_stoff;

  elsym = contents + bfd_xcoff_loader_symbol_offset (abfd, &ldhdr);

  elsymend = elsym + ldhdr.l_nsyms * bfd_xcoff_ldsymsz (abfd);
  for (; elsym < elsymend; elsym += bfd_xcoff_ldsymsz (abfd))
    {
      struct internal_ldsym ldsym;
      char nambuf[SYMNMLEN + 1];
      const char *name;
      struct bfd_link_hash_entry *h;

      bfd_xcoff_swap_ldsym_in (abfd, elsym, &ldsym);

      /* We are only interested in exported symbols.  */
      if ((ldsym.l_smtype & L_EXPORT) == 0)
	continue;

      if (ldsym._l._l_l._l_zeroes == 0)
	name = strings + ldsym._l._l_l._l_offset;
      else
	{
	  memcpy (nambuf, ldsym._l._l_name, SYMNMLEN);
	  nambuf[SYMNMLEN] = '\0';
	  name = nambuf;
	}

      h = bfd_link_hash_lookup (info->hash, name, false, false, true);

      /* We are only interested in symbols that are currently
	 undefined.  At this point we know that we are using an XCOFF
	 hash table.  */
      if (h != NULL
	  && h->type == bfd_link_hash_undefined
	  && (((struct xcoff_link_hash_entry *) h)->flags
	      & XCOFF_DEF_DYNAMIC) == 0)
	{
	  if (!(*info->callbacks
		->add_archive_element) (info, abfd, name, subsbfd))
	    continue;
	  *pneeded = true;
	  return true;
	}
    }

  /* We do not need this shared object's .loader section.  */
  free (contents);
  coff_section_data (abfd, lsec)->contents = NULL;

  return true;
}

/* Look through the symbols to see if this object file should be
   included in the link.  */

static bool
xcoff_link_check_ar_symbols (bfd *abfd,
			     struct bfd_link_info *info,
			     bool *pneeded,
			     bfd **subsbfd)
{
  bfd_size_type symesz;
  bfd_byte *esym;
  bfd_byte *esym_end;

  *pneeded = false;

  if ((abfd->flags & DYNAMIC) != 0
      && ! info->static_link
      && info->output_bfd->xvec == abfd->xvec)
    return xcoff_link_check_dynamic_ar_symbols (abfd, info, pneeded, subsbfd);

  symesz = bfd_coff_symesz (abfd);
  esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esym_end = esym + obj_raw_syment_count (abfd) * symesz;
  while (esym < esym_end)
    {
      struct internal_syment sym;

      bfd_coff_swap_sym_in (abfd, (void *) esym, (void *) &sym);
      esym += (sym.n_numaux + 1) * symesz;

      if (EXTERN_SYM_P (sym.n_sclass) && sym.n_scnum != N_UNDEF)
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];
	  struct bfd_link_hash_entry *h;

	  /* This symbol is externally visible, and is defined by this
	     object file.  */
	  name = _bfd_coff_internal_syment_name (abfd, &sym, buf);

	  if (name == NULL)
	    return false;
	  h = bfd_link_hash_lookup (info->hash, name, false, false, true);

	  /* We are only interested in symbols that are currently
	     undefined.  If a symbol is currently known to be common,
	     XCOFF linkers do not bring in an object file which
	     defines it.  We also don't bring in symbols to satisfy
	     undefined references in shared objects.  */
	  if (h != NULL
	      && h->type == bfd_link_hash_undefined
	      && (info->output_bfd->xvec != abfd->xvec
		  || (((struct xcoff_link_hash_entry *) h)->flags
		      & XCOFF_DEF_DYNAMIC) == 0))
	    {
	      if (!(*info->callbacks
		    ->add_archive_element) (info, abfd, name, subsbfd))
		continue;
	      *pneeded = true;
	      return true;
	    }
	}
    }

  /* We do not need this object file.  */
  return true;
}

/* Check a single archive element to see if we need to include it in
   the link.  *PNEEDED is set according to whether this element is
   needed in the link or not.  This is called via
   _bfd_generic_link_add_archive_symbols.  */

static bool
xcoff_link_check_archive_element (bfd *abfd,
				  struct bfd_link_info *info,
				  struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED,
				  const char *name ATTRIBUTE_UNUSED,
				  bool *pneeded)
{
  bool keep_syms_p;
  bfd *oldbfd;

  keep_syms_p = (obj_coff_external_syms (abfd) != NULL);
  if (!_bfd_coff_get_external_symbols (abfd))
    return false;

  oldbfd = abfd;
  if (!xcoff_link_check_ar_symbols (abfd, info, pneeded, &abfd))
    return false;

  if (*pneeded)
    {
      /* Potentially, the add_archive_element hook may have set a
	 substitute BFD for us.  */
      if (abfd != oldbfd)
	{
	  if (!keep_syms_p
	      && !_bfd_coff_free_symbols (oldbfd))
	    return false;
	  keep_syms_p = (obj_coff_external_syms (abfd) != NULL);
	  if (!_bfd_coff_get_external_symbols (abfd))
	    return false;
	}
      if (!xcoff_link_add_symbols (abfd, info))
	return false;
      if (info->keep_memory)
	keep_syms_p = true;
    }

  if (!keep_syms_p)
    {
      if (!_bfd_coff_free_symbols (abfd))
	return false;
    }

  return true;
}

/* Given an XCOFF BFD, add symbols to the global hash table as
   appropriate.  */

bool
_bfd_xcoff_bfd_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return xcoff_link_add_object_symbols (abfd, info);

    case bfd_archive:
      /* If the archive has a map, do the usual search.  We then need
	 to check the archive for dynamic objects, because they may not
	 appear in the archive map even though they should, perhaps, be
	 included.  If the archive has no map, we just consider each object
	 file in turn, since that apparently is what the AIX native linker
	 does.  */
      if (bfd_has_map (abfd))
	{
	  if (! (_bfd_generic_link_add_archive_symbols
		 (abfd, info, xcoff_link_check_archive_element)))
	    return false;
	}

      {
	bfd *member;

	member = bfd_openr_next_archived_file (abfd, NULL);
	while (member != NULL)
	  {
	    if (bfd_check_format (member, bfd_object)
		&& (info->output_bfd->xvec == member->xvec)
		&& (! bfd_has_map (abfd) || (member->flags & DYNAMIC) != 0))
	      {
		bool needed;

		if (! xcoff_link_check_archive_element (member, info,
							NULL, NULL, &needed))
		  return false;
		if (needed)
		  member->archive_pass = -1;
	      }
	    member = bfd_openr_next_archived_file (abfd, member);
	  }
      }

      return true;

    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

bool
_bfd_xcoff_define_common_symbol (bfd *output_bfd ATTRIBUTE_UNUSED,
				 struct bfd_link_info *info ATTRIBUTE_UNUSED,
				 struct bfd_link_hash_entry *harg)
{
  struct xcoff_link_hash_entry *h;

  if (!bfd_generic_define_common_symbol (output_bfd, info, harg))
    return false;

  h = (struct xcoff_link_hash_entry *) harg;
  h->flags |= XCOFF_DEF_REGULAR;
  return true;
}

/* If symbol H has not been interpreted as a function descriptor,
   see whether it should be.  Set up its descriptor information if so.  */

static bool
xcoff_find_function (struct bfd_link_info *info,
		     struct xcoff_link_hash_entry *h)
{
  if ((h->flags & XCOFF_DESCRIPTOR) == 0
      && h->root.root.string[0] != '.')
    {
      char *fnname;
      struct xcoff_link_hash_entry *hfn;
      size_t amt;

      amt = strlen (h->root.root.string) + 2;
      fnname = bfd_malloc (amt);
      if (fnname == NULL)
	return false;
      fnname[0] = '.';
      strcpy (fnname + 1, h->root.root.string);
      hfn = xcoff_link_hash_lookup (xcoff_hash_table (info),
				    fnname, false, false, true);
      free (fnname);
      if (hfn != NULL
	  && hfn->smclas == XMC_PR
	  && (hfn->root.type == bfd_link_hash_defined
	      || hfn->root.type == bfd_link_hash_defweak))
	{
	  h->flags |= XCOFF_DESCRIPTOR;
	  h->descriptor = hfn;
	  hfn->descriptor = h;
	}
    }
  return true;
}

/* Return true if the given bfd contains at least one shared object.  */

static bool
xcoff_archive_contains_shared_object_p (struct bfd_link_info *info,
					bfd *archive)
{
  struct xcoff_archive_info *archive_info;
  bfd *member;

  archive_info = xcoff_get_archive_info (info, archive);
  if (!archive_info->know_contains_shared_object_p)
    {
      member = bfd_openr_next_archived_file (archive, NULL);
      while (member != NULL && (member->flags & DYNAMIC) == 0)
	member = bfd_openr_next_archived_file (archive, member);

      archive_info->contains_shared_object_p = (member != NULL);
      archive_info->know_contains_shared_object_p = 1;
    }
  return archive_info->contains_shared_object_p;
}

/* Symbol H qualifies for export by -bexpfull.  Return true if it also
   qualifies for export by -bexpall.  */

static bool
xcoff_covered_by_expall_p (struct xcoff_link_hash_entry *h)
{
  /* Exclude symbols beginning with '_'.  */
  if (h->root.root.string[0] == '_')
    return false;

  /* Exclude archive members that would otherwise be unreferenced.  */
  if ((h->flags & XCOFF_MARK) == 0
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && h->root.u.def.section->owner != NULL
      && h->root.u.def.section->owner->my_archive != NULL)
    return false;

  return true;
}

/* Return true if symbol H qualifies for the forms of automatic export
   specified by AUTO_EXPORT_FLAGS.  */

static bool
xcoff_auto_export_p (struct bfd_link_info *info,
		     struct xcoff_link_hash_entry *h,
		     unsigned int auto_export_flags)
{
  /* Don't automatically export things that were explicitly exported.  */
  if ((h->flags & XCOFF_EXPORT) != 0)
    return false;

  /* Don't export things that we don't define.  */
  if ((h->flags & XCOFF_DEF_REGULAR) == 0)
    return false;

  /* Don't export functions; export their descriptors instead.  */
  if (h->root.root.string[0] == '.')
    return false;

  /* Don't export hidden or internal symbols.  */
  if (h->visibility == SYM_V_HIDDEN
      || h->visibility == SYM_V_INTERNAL)
    return false;

  /* We don't export a symbol which is being defined by an object
     included from an archive which contains a shared object.  The
     rationale is that if an archive contains both an unshared and
     a shared object, then there must be some reason that the
     unshared object is unshared, and we don't want to start
     providing a shared version of it.  In particular, this solves
     a bug involving the _savefNN set of functions.  gcc will call
     those functions without providing a slot to restore the TOC,
     so it is essential that these functions be linked in directly
     and not from a shared object, which means that a shared
     object which also happens to link them in must not export
     them.  This is confusing, but I haven't been able to think of
     a different approach.  Note that the symbols can, of course,
     be exported explicitly.  */
  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    {
      bfd *owner;

      owner = h->root.u.def.section->owner;
      if (owner != NULL
	  && owner->my_archive != NULL
	  && xcoff_archive_contains_shared_object_p (info, owner->my_archive))
	return false;
    }

  /* Otherwise, all symbols are exported by -bexpfull.  */
  if ((auto_export_flags & XCOFF_EXPFULL) != 0)
    return true;

  /* Despite its name, -bexpall exports most but not all symbols.  */
  if ((auto_export_flags & XCOFF_EXPALL) != 0
      && xcoff_covered_by_expall_p (h))
    return true;

  return false;
}

/* Return true if relocation REL needs to be copied to the .loader section.
   If REL is against a global symbol, H is that symbol, otherwise it
   is null.  */

static bool
xcoff_need_ldrel_p (struct bfd_link_info *info, struct internal_reloc *rel,
		    struct xcoff_link_hash_entry *h, asection *ssec)
{
  if (!xcoff_hash_table (info)->loader_section)
    return false;

  switch (rel->r_type)
    {
    case R_TOC:
    case R_GL:
    case R_TCL:
    case R_TRL:
    case R_TRLA:
      /* We should never need a .loader reloc for a TOC-relative reloc.  */
      return false;

    default:
      /* In this case, relocations against defined symbols can be resolved
	 statically.  */
      if (h == NULL
	  || h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak
	  || h->root.type == bfd_link_hash_common)
	return false;

      /* We will always provide a local definition of function symbols,
	 even if we don't have one yet.  */
      if ((h->flags & XCOFF_CALLED) != 0)
	return false;

      return true;

    case R_POS:
    case R_NEG:
    case R_RL:
    case R_RLA:
      /* Absolute relocations against absolute symbols can be
	 resolved statically.  */
      if (h != NULL
	  && (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && !h->root.rel_from_abs)
	{
	  asection *sec = h->root.u.def.section;
	  if (bfd_is_abs_section (sec)
	      || (sec != NULL
		  && bfd_is_abs_section (sec->output_section)))
	    return false;
	}

      /* Absolute relocations from read-only sections are forbidden
	 by AIX loader. However, they can appear in their section's
         relocations.  */
      if (ssec != NULL
	  && (ssec->output_section->flags & SEC_READONLY) != 0)
	return false;

      return true;

    case R_TLS:
    case R_TLS_LE:
    case R_TLS_IE:
    case R_TLS_LD:
    case R_TLSM:
    case R_TLSML:
      return true;
    }
}

/* Mark a symbol as not being garbage, including the section in which
   it is defined.  */

static inline bool
xcoff_mark_symbol (struct bfd_link_info *info, struct xcoff_link_hash_entry *h)
{
  if ((h->flags & XCOFF_MARK) != 0)
    return true;

  h->flags |= XCOFF_MARK;

  /* If we're marking an undefined symbol, try find some way of
     defining it.  */
  if (!bfd_link_relocatable (info)
      && (h->flags & XCOFF_IMPORT) == 0
      && (h->flags & XCOFF_DEF_REGULAR) == 0
      && (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak))
    {
      /* First check whether this symbol can be interpreted as an
	 undefined function descriptor for a defined function symbol.  */
      if (!xcoff_find_function (info, h))
	return false;

      if ((h->flags & XCOFF_DESCRIPTOR) != 0
	  && (h->descriptor->root.type == bfd_link_hash_defined
	      || h->descriptor->root.type == bfd_link_hash_defweak))
	{
	  /* This is a descriptor for a defined symbol, but the input
	     objects have not defined the descriptor itself.  Fill in
	     the definition automatically.

	     Note that we do this even if we found a dynamic definition
	     of H.  The local function definition logically overrides
	     the dynamic one.  */
	  asection *sec;

	  sec = xcoff_hash_table (info)->descriptor_section;
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = sec;
	  h->root.u.def.value = sec->size;
	  h->smclas = XMC_DS;
	  h->flags |= XCOFF_DEF_REGULAR;

	  /* The size of the function descriptor depends on whether this
	     is xcoff32 (12) or xcoff64 (24).  */
	  sec->size += bfd_xcoff_function_descriptor_size (sec->owner);

	  /* A function descriptor uses two relocs: one for the
	     associated code, and one for the TOC address.  */
	  xcoff_hash_table (info)->ldinfo.ldrel_count += 2;
	  sec->reloc_count += 2;

	  /* Mark the function itself.  */
	  if (!xcoff_mark_symbol (info, h->descriptor))
	    return false;

	  /* Mark the TOC section, so that we get an anchor
	     to relocate against.  */
	  if (!xcoff_mark (info, xcoff_hash_table (info)->toc_section))
	    return false;

	  /* We handle writing out the contents of the descriptor in
	     xcoff_write_global_symbol.  */
	}
      else if (info->static_link)
	/* We can't get a symbol value dynamically, so just assume
	   that it's undefined.  */
	h->flags |= XCOFF_WAS_UNDEFINED;
      else if ((h->flags & XCOFF_CALLED) != 0)
	{
	  /* This is a function symbol for which we need to create
	     linkage code.  */
	  asection *sec;
	  struct xcoff_link_hash_entry *hds;

	  /* Mark the descriptor (and its TOC section).  */
	  hds = h->descriptor;
	  BFD_ASSERT ((hds->root.type == bfd_link_hash_undefined
		       || hds->root.type == bfd_link_hash_undefweak)
		      && (hds->flags & XCOFF_DEF_REGULAR) == 0);
	  if (!xcoff_mark_symbol (info, hds))
	    return false;

	  /* Treat this symbol as undefined if the descriptor was.  */
	  if ((hds->flags & XCOFF_WAS_UNDEFINED) != 0)
	    h->flags |= XCOFF_WAS_UNDEFINED;

	  /* Allocate room for the global linkage code itself.  */
	  sec = xcoff_hash_table (info)->linkage_section;
	  h->root.type = bfd_link_hash_defined;
	  h->root.u.def.section = sec;
	  h->root.u.def.value = sec->size;
	  h->smclas = XMC_GL;
	  h->flags |= XCOFF_DEF_REGULAR;
	  sec->size += bfd_xcoff_glink_code_size (info->output_bfd);

	  /* The global linkage code requires a TOC entry for the
	     descriptor.  */
	  if (hds->toc_section == NULL)
	    {
	      int byte_size;

	      /* 32 vs 64
		 xcoff32 uses 4 bytes in the toc.
		 xcoff64 uses 8 bytes in the toc.  */
	      if (bfd_xcoff_is_xcoff64 (info->output_bfd))
		byte_size = 8;
	      else if (bfd_xcoff_is_xcoff32 (info->output_bfd))
		byte_size = 4;
	      else
		return false;

	      /* Allocate room in the fallback TOC section.  */
	      hds->toc_section = xcoff_hash_table (info)->toc_section;
	      hds->u.toc_offset = hds->toc_section->size;
	      hds->toc_section->size += byte_size;
	      if (!xcoff_mark (info, hds->toc_section))
		return false;

	      /* Allocate room for a static and dynamic R_TOC
		 relocation.  */
	      ++xcoff_hash_table (info)->ldinfo.ldrel_count;
	      ++hds->toc_section->reloc_count;

	      /* Set the index to -2 to force this symbol to
		 get written out.  */
	      hds->indx = -2;
	      hds->flags |= XCOFF_SET_TOC | XCOFF_LDREL;
	    }
	}
      else if ((h->flags & XCOFF_DEF_DYNAMIC) == 0)
	{
	  /* Record that the symbol was undefined, then import it.
	     -brtl links use a special fake import file.  */
	  h->flags |= XCOFF_WAS_UNDEFINED | XCOFF_IMPORT;
	  if (xcoff_hash_table (info)->rtld)
	    {
	      if (!xcoff_set_import_path (info, h, "", "..", ""))
		return false;
	    }
	  else
	    {
	      if (!xcoff_set_import_path (info, h, NULL, NULL, NULL))
		return false;
	    }
	}
    }

  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    {
      asection *hsec;

      hsec = h->root.u.def.section;
      if (! bfd_is_abs_section (hsec)
	  && hsec->gc_mark == 0)
	{
	  if (! xcoff_mark (info, hsec))
	    return false;
	}
    }

  if (h->toc_section != NULL
      && h->toc_section->gc_mark == 0)
    {
      if (! xcoff_mark (info, h->toc_section))
	return false;
    }

  return true;
}

/* Look for a symbol called NAME.  If the symbol is defined, mark it.
   If the symbol exists, set FLAGS.  */

static bool
xcoff_mark_symbol_by_name (struct bfd_link_info *info,
			   const char *name, unsigned int flags)
{
  struct xcoff_link_hash_entry *h;

  h = xcoff_link_hash_lookup (xcoff_hash_table (info), name,
			      false, false, true);
  if (h != NULL)
    {
      h->flags |= flags;
      if (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
	{
	  if (!xcoff_mark (info, h->root.u.def.section))
	    return false;
	}
    }
  return true;
}

/* The mark phase of garbage collection.  For a given section, mark
   it, and all the sections which define symbols to which it refers.
   Because this function needs to look at the relocs, we also count
   the number of relocs which need to be copied into the .loader
   section.  */

static bool
xcoff_mark (struct bfd_link_info *info, asection *sec)
{
  if (bfd_is_const_section (sec)
      || sec->gc_mark != 0)
    return true;

  sec->gc_mark = 1;

  if (sec->owner->xvec != info->output_bfd->xvec)
    return true;

  if (coff_section_data (sec->owner, sec) == NULL)
    return true;


  if (xcoff_section_data (sec->owner, sec) != NULL)
    {
      struct xcoff_link_hash_entry **syms;
      asection **csects;
      unsigned long i, first, last;

      /* Mark all the symbols in this section.  */
      syms = obj_xcoff_sym_hashes (sec->owner);
      csects = xcoff_data (sec->owner)->csects;
      first = xcoff_section_data (sec->owner, sec)->first_symndx;
      last = xcoff_section_data (sec->owner, sec)->last_symndx;
      for (i = first; i <= last; i++)
	if (csects[i] == sec
	    && syms[i] != NULL
	    && (syms[i]->flags & XCOFF_MARK) == 0)
	  {
	    if (!xcoff_mark_symbol (info, syms[i]))
	      return false;
	  }
    }

  /* Look through the section relocs.  */
  if ((sec->flags & SEC_RELOC) != 0
      && sec->reloc_count > 0)
    {
      struct internal_reloc *rel, *relend;

      rel = xcoff_read_internal_relocs (sec->owner, sec, true,
					NULL, false, NULL);
      if (rel == NULL)
	return false;
      relend = rel + sec->reloc_count;
      for (; rel < relend; rel++)
	{
	  struct xcoff_link_hash_entry *h;

	  if ((unsigned int) rel->r_symndx
	      > obj_raw_syment_count (sec->owner))
	    continue;

	  h = obj_xcoff_sym_hashes (sec->owner)[rel->r_symndx];
	  if (h != NULL)
	    {
	      if ((h->flags & XCOFF_MARK) == 0)
		{
		  if (!xcoff_mark_symbol (info, h))
		    return false;
		}
	    }
	  else
	    {
	      asection *rsec;

	      rsec = xcoff_data (sec->owner)->csects[rel->r_symndx];
	      if (rsec != NULL
		  && rsec->gc_mark == 0)
		{
		  if (!xcoff_mark (info, rsec))
		    return false;
		}
	    }

	  /* See if this reloc needs to be copied into the .loader
	     section.  */
	  if ((sec->flags & SEC_DEBUGGING) == 0
	      && xcoff_need_ldrel_p (info, rel, h, sec))
	    {
	      ++xcoff_hash_table (info)->ldinfo.ldrel_count;
	      if (h != NULL)
		h->flags |= XCOFF_LDREL;
	    }
	}

      if (! info->keep_memory
	  && coff_section_data (sec->owner, sec) != NULL)
	{
	  free (coff_section_data (sec->owner, sec)->relocs);
	  coff_section_data (sec->owner, sec)->relocs = NULL;
	}
    }

  return true;
}

/* Routines that are called after all the input files have been
   handled, but before the sections are laid out in memory.  */

/* The sweep phase of garbage collection.  Remove all garbage
   sections.  */

static void
xcoff_sweep (struct bfd_link_info *info)
{
  bfd *sub;

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *o;
      bool some_kept = false;

      /* As says below keep all sections from non-XCOFF
         input files.  */
      if (sub->xvec != info->output_bfd->xvec)
	some_kept = true;
      else
	{
	  /* See whether any section is already marked.  */
	  for (o = sub->sections; o != NULL; o = o->next)
	    if (o->gc_mark)
	      some_kept = true;
	}

      /* If no section in this file will be kept, then we can
	 toss out debug sections.  */
      if (!some_kept)
	{
	  for (o = sub->sections; o != NULL; o = o->next)
	    {
	      o->size = 0;
	      o->reloc_count = 0;
	    }
	  continue;
	}

      /* Keep all sections from non-XCOFF input files.  Keep
	 special sections.  Keep .debug sections for the
	 moment.  */
      for (o = sub->sections; o != NULL; o = o->next)
	{
	  if (o->gc_mark == 1)
	    continue;

	  if (sub->xvec != info->output_bfd->xvec
	      || o == xcoff_hash_table (info)->debug_section
	      || o == xcoff_hash_table (info)->loader_section
	      || o == xcoff_hash_table (info)->linkage_section
	      || o == xcoff_hash_table (info)->descriptor_section
	      || (bfd_section_flags (o) & SEC_DEBUGGING)
	      || strcmp (o->name, ".debug") == 0)
	    xcoff_mark (info, o);
	  else
	    {
	      o->size = 0;
	      o->reloc_count = 0;
	    }
	}
    }
}

/* Initialize the back-end with linker infos.  */

bool
bfd_xcoff_link_init (struct bfd_link_info *info,
		     struct bfd_xcoff_link_params *params)
{
  xcoff_hash_table (info)->params = params;

  return true;
}

/* Record the number of elements in a set.  This is used to output the
   correct csect length.  */

bool
bfd_xcoff_link_record_set (bfd *output_bfd,
			   struct bfd_link_info *info,
			   struct bfd_link_hash_entry *harg,
			   bfd_size_type size)
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;
  struct xcoff_link_size_list *n;
  size_t amt;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    return true;

  /* This will hardly ever be called.  I don't want to burn four bytes
     per global symbol, so instead the size is kept on a linked list
     attached to the hash table.  */
  amt = sizeof (* n);
  n = bfd_alloc (output_bfd, amt);
  if (n == NULL)
    return false;
  n->next = xcoff_hash_table (info)->size_list;
  n->h = h;
  n->size = size;
  xcoff_hash_table (info)->size_list = n;

  h->flags |= XCOFF_HAS_SIZE;

  return true;
}

/* Import a symbol.  */

bool
bfd_xcoff_import_symbol (bfd *output_bfd,
			 struct bfd_link_info *info,
			 struct bfd_link_hash_entry *harg,
			 bfd_vma val,
			 const char *imppath,
			 const char *impfile,
			 const char *impmember,
			 unsigned int syscall_flag)
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    return true;

  /* A symbol name which starts with a period is the code for a
     function.  If the symbol is undefined, then add an undefined
     symbol for the function descriptor, and import that instead.  */
  if (h->root.root.string[0] == '.'
      && h->root.type == bfd_link_hash_undefined
      && val == (bfd_vma) -1)
    {
      struct xcoff_link_hash_entry *hds;

      hds = h->descriptor;
      if (hds == NULL)
	{
	  hds = xcoff_link_hash_lookup (xcoff_hash_table (info),
					h->root.root.string + 1,
					true, false, true);
	  if (hds == NULL)
	    return false;
	  if (hds->root.type == bfd_link_hash_new)
	    {
	      hds->root.type = bfd_link_hash_undefined;
	      hds->root.u.undef.abfd = h->root.u.undef.abfd;
	    }
	  hds->flags |= XCOFF_DESCRIPTOR;
	  BFD_ASSERT ((h->flags & XCOFF_DESCRIPTOR) == 0);
	  hds->descriptor = h;
	  h->descriptor = hds;
	}

      /* Now, if the descriptor is undefined, import the descriptor
	 rather than the symbol we were told to import.  FIXME: Is
	 this correct in all cases?  */
      if (hds->root.type == bfd_link_hash_undefined)
	h = hds;
    }

  h->flags |= (XCOFF_IMPORT | syscall_flag);

  if (val != (bfd_vma) -1)
    {
      if (h->root.type == bfd_link_hash_defined)
	(*info->callbacks->multiple_definition) (info, &h->root, output_bfd,
						 bfd_abs_section_ptr, val);

      h->root.type = bfd_link_hash_defined;
      h->root.u.def.section = bfd_abs_section_ptr;
      h->root.u.def.value = val;
      h->smclas = XMC_XO;
    }

  if (!xcoff_set_import_path (info, h, imppath, impfile, impmember))
    return false;

  return true;
}

/* Export a symbol.  */

bool
bfd_xcoff_export_symbol (bfd *output_bfd,
			 struct bfd_link_info *info,
			 struct bfd_link_hash_entry *harg)
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) harg;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    return true;

  /* As AIX linker, symbols exported with hidden visibility are
     silently ignored.  */
  if (h->visibility == SYM_V_HIDDEN)
    return true;

  if (h->visibility == SYM_V_INTERNAL)
    {
      _bfd_error_handler (_("%pB: cannot export internal symbol `%s`."),
			  output_bfd, h->root.root.string);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  h->flags |= XCOFF_EXPORT;

  /* FIXME: I'm not at all sure what syscall is supposed to mean, so
     I'm just going to ignore it until somebody explains it.  */

  /* Make sure we don't garbage collect this symbol.  */
  if (! xcoff_mark_symbol (info, h))
    return false;

  /* If this is a function descriptor, make sure we don't garbage
     collect the associated function code.  We normally don't have to
     worry about this, because the descriptor will be attached to a
     section with relocs, but if we are creating the descriptor
     ourselves those relocs will not be visible to the mark code.  */
  if ((h->flags & XCOFF_DESCRIPTOR) != 0)
    {
      if (! xcoff_mark_symbol (info, h->descriptor))
	return false;
    }

  return true;
}

/* Count a reloc against a symbol.  This is called for relocs
   generated by the linker script, typically for global constructors
   and destructors.  */

bool
bfd_xcoff_link_count_reloc (bfd *output_bfd,
			    struct bfd_link_info *info,
			    const char *name)
{
  struct xcoff_link_hash_entry *h;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    return true;

  h = ((struct xcoff_link_hash_entry *)
       bfd_wrapped_link_hash_lookup (output_bfd, info, name, false, false,
				     false));
  if (h == NULL)
    {
      _bfd_error_handler (_("%s: no such symbol"), name);
      bfd_set_error (bfd_error_no_symbols);
      return false;
    }

  h->flags |= XCOFF_REF_REGULAR;
  if (xcoff_hash_table (info)->loader_section)
    {
      h->flags |= XCOFF_LDREL;
      ++xcoff_hash_table (info)->ldinfo.ldrel_count;
    }

  /* Mark the symbol to avoid garbage collection.  */
  if (! xcoff_mark_symbol (info, h))
    return false;

  return true;
}

/* This function is called for each symbol to which the linker script
   assigns a value.
   FIXME: In cases like the linker test ld-scripts/defined5 where a
   symbol is defined both by an input object file and the script,
   the script definition doesn't override the object file definition
   as is usual for other targets.  At least not when the symbol is
   output.  Other uses of the symbol value by the linker do use the
   script value.  */

bool
bfd_xcoff_record_link_assignment (bfd *output_bfd,
				  struct bfd_link_info *info,
				  const char *name)
{
  struct xcoff_link_hash_entry *h;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    return true;

  h = xcoff_link_hash_lookup (xcoff_hash_table (info), name, true, true,
			      false);
  if (h == NULL)
    return false;

  h->flags |= XCOFF_DEF_REGULAR;

  return true;
}

/* An xcoff_link_hash_traverse callback for which DATA points to an
   xcoff_loader_info.  Mark all symbols that should be automatically
   exported.  */

static bool
xcoff_mark_auto_exports (struct xcoff_link_hash_entry *h, void *data)
{
  struct xcoff_loader_info *ldinfo;

  ldinfo = (struct xcoff_loader_info *) data;
  if (xcoff_auto_export_p (ldinfo->info, h, ldinfo->auto_export_flags))
    {
      if (!xcoff_mark_symbol (ldinfo->info, h))
	ldinfo->failed = true;
    }
  return true;
}

/* INPUT_BFD has an external symbol associated with hash table entry H
   and csect CSECT.   Return true if INPUT_BFD defines H.  */

static bool
xcoff_final_definition_p (bfd *input_bfd, struct xcoff_link_hash_entry *h,
			  asection *csect)
{
  switch (h->root.type)
    {
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      /* No input bfd owns absolute symbols.  They are written by
	 xcoff_write_global_symbol instead.  */
      return (!bfd_is_abs_section (csect)
	      && h->root.u.def.section == csect);

    case bfd_link_hash_common:
      return h->root.u.c.p->section->owner == input_bfd;

    case bfd_link_hash_undefined:
    case bfd_link_hash_undefweak:
      /* We can't treat undef.abfd as the owner because that bfd
	 might be a dynamic object.  Allow any bfd to claim it.  */
      return true;

    default:
      abort ();
    }
}

/* See if H should have a loader symbol associated with it.  */

static bool
xcoff_build_ldsym (struct xcoff_loader_info *ldinfo,
		   struct xcoff_link_hash_entry *h)
{
  size_t amt;

  /* Warn if this symbol is exported but not defined.  */
  if ((h->flags & XCOFF_EXPORT) != 0
      && (h->flags & XCOFF_WAS_UNDEFINED) != 0)
    {
      _bfd_error_handler
	(_("warning: attempt to export undefined symbol `%s'"),
	 h->root.root.string);
      return true;
    }

  /* We need to add a symbol to the .loader section if it is mentioned
     in a reloc which we are copying to the .loader section and it was
     not defined or common, or if it is the entry point, or if it is
     being exported.  */
  if (((h->flags & XCOFF_LDREL) == 0
       || h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak
       || h->root.type == bfd_link_hash_common)
      && (h->flags & XCOFF_ENTRY) == 0
      && (h->flags & XCOFF_EXPORT) == 0)
    return true;

  /* We need to add this symbol to the .loader symbols.  */

  BFD_ASSERT (h->ldsym == NULL);
  amt = sizeof (struct internal_ldsym);
  h->ldsym = bfd_zalloc (ldinfo->output_bfd, amt);
  if (h->ldsym == NULL)
    {
      ldinfo->failed = true;
      return false;
    }

  if ((h->flags & XCOFF_IMPORT) != 0)
    {
      /* Give imported descriptors class XMC_DS rather than XMC_UA.  */
      if ((h->flags & XCOFF_DESCRIPTOR) != 0)
	h->smclas = XMC_DS;
      h->ldsym->l_ifile = h->ldindx;
    }

  /* The first 3 symbol table indices are reserved to indicate the
     data, text and bss sections.  */
  h->ldindx = ldinfo->ldsym_count + 3;

  ++ldinfo->ldsym_count;

  if (! bfd_xcoff_put_ldsymbol_name (ldinfo->output_bfd, ldinfo,
				     h->ldsym, h->root.root.string))
    return false;

  h->flags |= XCOFF_BUILT_LDSYM;
  return true;
}

/* An xcoff_htab_traverse callback that is called for each symbol
   once garbage collection is complete.  */

static bool
xcoff_post_gc_symbol (struct xcoff_link_hash_entry *h, void * p)
{
  struct xcoff_loader_info *ldinfo = (struct xcoff_loader_info *) p;

  /* __rtinit, this symbol has special handling. */
  if (h->flags & XCOFF_RTINIT)
    return true;

  /* We don't want to garbage collect symbols which are not defined in
     XCOFF files.  This is a convenient place to mark them.  */
  if (xcoff_hash_table (ldinfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && (h->root.u.def.section->owner == NULL
	  || (h->root.u.def.section->owner->xvec
	      != ldinfo->info->output_bfd->xvec)))
    h->flags |= XCOFF_MARK;

  /* Skip discarded symbols.  */
  if (xcoff_hash_table (ldinfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0)
    return true;

  /* If this is still a common symbol, and it wasn't garbage
     collected, we need to actually allocate space for it in the .bss
     section.  */
  if (h->root.type == bfd_link_hash_common
      && h->root.u.c.p->section->size == 0)
    {
      BFD_ASSERT (bfd_is_com_section (h->root.u.c.p->section));
      h->root.u.c.p->section->size = h->root.u.c.size;
    }

  if (xcoff_hash_table (ldinfo->info)->loader_section)
    {
      if (xcoff_auto_export_p (ldinfo->info, h, ldinfo->auto_export_flags))
	h->flags |= XCOFF_EXPORT;

      if (!xcoff_build_ldsym (ldinfo, h))
	return false;
    }

  return true;
}

/* INPUT_BFD includes XCOFF symbol ISYM, which is associated with linker
   hash table entry H and csect CSECT.  AUX contains ISYM's auxiliary
   csect information, if any.  NAME is the function's name if the name
   is stored in the .debug section, otherwise it is null.

   Return 1 if we should include an appropriately-adjusted ISYM
   in the output file, 0 if we should discard ISYM, or -1 if an
   error occured.  */

static int
xcoff_keep_symbol_p (struct bfd_link_info *info, bfd *input_bfd,
		     struct internal_syment *isym,
		     union internal_auxent *aux,
		     struct xcoff_link_hash_entry *h,
		     asection *csect, const char *name)
{
  int smtyp;

  /* If we are skipping this csect, we want to strip the symbol too.  */
  if (csect == NULL)
    return 0;

  /* Likewise if we garbage-collected the csect.  */
  if (xcoff_hash_table (info)->gc
      && !bfd_is_abs_section (csect)
      && !bfd_is_und_section (csect)
      && csect->gc_mark == 0)
    return 0;

  /* An XCOFF linker always removes C_STAT symbols.  */
  if (isym->n_sclass == C_STAT)
    return 0;

  /* We generate the TOC anchor separately.  */
  if (isym->n_sclass == C_HIDEXT
      && aux->x_csect.x_smclas == XMC_TC0)
    return 0;

  /* If we are stripping all symbols, we want to discard this one.  */
  if (info->strip == strip_all)
    return 0;

  /* Discard symbols that are defined elsewhere.  */
  if (EXTERN_SYM_P (isym->n_sclass))
    {
      if ((h->flags & XCOFF_ALLOCATED) != 0)
	return 0;
      if (!xcoff_final_definition_p (input_bfd, h, csect))
	return 0;
    }

  /* If we're discarding local symbols, check whether ISYM is local.  */
  smtyp = SMTYP_SMTYP (aux->x_csect.x_smtyp);
  if (info->discard == discard_all
      && !EXTERN_SYM_P (isym->n_sclass)
      && (isym->n_sclass != C_HIDEXT || smtyp != XTY_SD))
    return 0;

  /* If we're stripping debugging symbols, check whether ISYM is one.  */
  if (info->strip == strip_debugger
      && isym->n_scnum == N_DEBUG)
    return 0;

  /* If we are stripping symbols based on name, check how ISYM's
     name should be handled.  */
  if (info->strip == strip_some
      || info->discard == discard_l)
    {
      char buf[SYMNMLEN + 1];

      if (name == NULL)
	{
	  name = _bfd_coff_internal_syment_name (input_bfd, isym, buf);
	  if (name == NULL)
	    return -1;
	}

      if (info->strip == strip_some
	  && bfd_hash_lookup (info->keep_hash, name, false, false) == NULL)
	return 0;

      if (info->discard == discard_l
	  && !EXTERN_SYM_P (isym->n_sclass)
	  && (isym->n_sclass != C_HIDEXT || smtyp != XTY_SD)
	  && bfd_is_local_label_name (input_bfd, name))
	return 0;
    }

  return 1;
}

/* Compute the current size of the .loader section. Start filling
   its header but it will be finalized in xcoff_build_loader_section.   */

static bool
xcoff_size_loader_section (struct xcoff_loader_info *ldinfo)
{
  bfd *output_bfd;
  struct xcoff_link_hash_table *htab;
  struct internal_ldhdr *ldhdr;
  struct xcoff_import_file *fl;
  bfd_size_type stoff;
  size_t impsize, impcount;
  asection *lsec;

  output_bfd = ldinfo->output_bfd;
  htab = xcoff_hash_table (ldinfo->info);
  ldhdr = &htab->ldhdr;

  /* If this function has already been called (ie l_version is set)
     and the number of symbols or relocations haven't changed since
     last call, the size is already known.  */
  if (ldhdr->l_version != 0
      && ldhdr->l_nsyms == ldinfo->ldsym_count
      && ldhdr->l_nreloc == ldinfo->ldrel_count)
    return true;

  /* Work out the size of the import file names.  Each import file ID
     consists of three null terminated strings: the path, the file
     name, and the archive member name.  The first entry in the list
     of names is the path to use to find objects, which the linker has
     passed in as the libpath argument.  For some reason, the path
     entry in the other import file names appears to always be empty.  */
  if (ldhdr->l_nimpid == 0)
    {
      impsize = strlen (ldinfo->libpath) + 3;
      impcount = 1;
      for (fl = htab->imports; fl != NULL; fl = fl->next)
	{
	  ++impcount;
	  impsize += (strlen (fl->path)
		      + strlen (fl->file)
		      + strlen (fl->member)
		      + 3);
	}
      ldhdr->l_istlen = impsize;
      ldhdr->l_nimpid = impcount;
    }

  /* Set up the .loader section header.  */
  ldhdr->l_version = bfd_xcoff_ldhdr_version(output_bfd);
  ldhdr->l_nsyms = ldinfo->ldsym_count;
  ldhdr->l_nreloc = ldinfo->ldrel_count;
  ldhdr->l_impoff = (bfd_xcoff_ldhdrsz (output_bfd)
		     + ldhdr->l_nsyms * bfd_xcoff_ldsymsz (output_bfd)
		     + ldhdr->l_nreloc * bfd_xcoff_ldrelsz (output_bfd));
  ldhdr->l_stlen = ldinfo->string_size;
  stoff = ldhdr->l_impoff + ldhdr->l_istlen;
  if (ldinfo->string_size == 0)
    ldhdr->l_stoff = 0;
  else
    ldhdr->l_stoff = stoff;

  /* 64 bit elements to ldhdr
     The swap out routine for 32 bit will ignore them.
     Nothing fancy, symbols come after the header and relocs come
     after symbols.  */
  ldhdr->l_symoff = bfd_xcoff_ldhdrsz (output_bfd);
  ldhdr->l_rldoff = (bfd_xcoff_ldhdrsz (output_bfd)
		     + ldhdr->l_nsyms * bfd_xcoff_ldsymsz (output_bfd));

  /* Save the size of the .loader section.  */
  lsec = htab->loader_section;
  lsec->size = stoff + ldhdr->l_stlen;

  return true;
}

/* Prepare the .loader section.  This is called by the XCOFF linker
   emulation before_allocation routine.  We must set the size of the
   .loader section before the linker lays out the output file.  However,
   some symbols or relocations might be append to the .loader section
   when processing the addresses, thus it's not layout right now and
   its size might change.
   LIBPATH is the library path to search for shared objects; this is
   normally built from the -L arguments passed to the linker.  ENTRY
   is the name of the entry point symbol (the -e linker option).
   FILE_ALIGN is the alignment to use for sections within the file
   (the -H linker option).  MAXSTACK is the maximum stack size (the
   -bmaxstack linker option).  MAXDATA is the maximum data size (the
   -bmaxdata linker option).  GC is whether to do garbage collection
   (the -bgc linker option).  MODTYPE is the module type (the
   -bmodtype linker option).  TEXTRO is whether the text section must
   be read only (the -btextro linker option).  AUTO_EXPORT_FLAGS
   is a mask of XCOFF_EXPALL and XCOFF_EXPFULL.  SPECIAL_SECTIONS
   is set by this routine to csects with magic names like _end.  */

bool
bfd_xcoff_size_dynamic_sections (bfd *output_bfd,
				 struct bfd_link_info *info,
				 const char *libpath,
				 const char *entry,
				 unsigned long file_align,
				 unsigned long maxstack,
				 unsigned long maxdata,
				 bool gc,
				 int modtype,
				 bool textro,
				 unsigned int auto_export_flags,
				 asection **special_sections,
				 bool rtld)
{
  struct xcoff_loader_info *ldinfo;
  int i;
  asection *sec;
  bfd *sub;
  size_t amt;

  if (bfd_get_flavour (output_bfd) != bfd_target_xcoff_flavour)
    {
      for (i = 0; i < XCOFF_NUMBER_OF_SPECIAL_SECTIONS; i++)
	special_sections[i] = NULL;
      return true;
    }

  /* Setup ldinfo.  */
  ldinfo = &(xcoff_hash_table (info)->ldinfo);

  ldinfo->failed = false;
  ldinfo->output_bfd = output_bfd;
  ldinfo->info = info;
  ldinfo->auto_export_flags = auto_export_flags;
  ldinfo->ldsym_count = 0;
  ldinfo->string_size = 0;
  ldinfo->strings = NULL;
  ldinfo->string_alc = 0;
  ldinfo->libpath = libpath;

  xcoff_data (output_bfd)->maxstack = maxstack;
  xcoff_data (output_bfd)->maxdata = maxdata;
  xcoff_data (output_bfd)->modtype = modtype;

  xcoff_hash_table (info)->file_align = file_align;
  xcoff_hash_table (info)->textro = textro;
  xcoff_hash_table (info)->rtld = rtld;

  /* __rtinit */
  if (xcoff_hash_table (info)->loader_section
      && (info->init_function || info->fini_function || rtld))
    {
      struct xcoff_link_hash_entry *hsym;
      struct internal_ldsym *ldsym;

      hsym = xcoff_link_hash_lookup (xcoff_hash_table (info),
				     "__rtinit", false, false, true);
      if (hsym == NULL)
	{
	  _bfd_error_handler
	    (_("error: undefined symbol __rtinit"));
	  return false;
	}

      xcoff_mark_symbol (info, hsym);
      hsym->flags |= (XCOFF_DEF_REGULAR | XCOFF_RTINIT);

      /* __rtinit initialized.  */
      amt = sizeof (* ldsym);
      ldsym = bfd_malloc (amt);

      ldsym->l_value = 0;		/* Will be filled in later.  */
      ldsym->l_scnum = 2;		/* Data section.  */
      ldsym->l_smtype = XTY_SD;		/* Csect section definition.  */
      ldsym->l_smclas = 5;		/* .rw.  */
      ldsym->l_ifile = 0;		/* Special system loader symbol.  */
      ldsym->l_parm = 0;		/* NA.  */

      /* Force __rtinit to be the first symbol in the loader symbol table
	 See xcoff_build_ldsyms

	 The first 3 symbol table indices are reserved to indicate the data,
	 text and bss sections.  */
      BFD_ASSERT (0 == ldinfo->ldsym_count);

      hsym->ldindx = 3;
      ldinfo->ldsym_count = 1;
      hsym->ldsym = ldsym;

      if (! bfd_xcoff_put_ldsymbol_name (ldinfo->output_bfd, ldinfo,
					 hsym->ldsym, hsym->root.root.string))
	return false;

      /* This symbol is written out by xcoff_write_global_symbol
	 Set stuff up so xcoff_write_global_symbol logic works.  */
      hsym->flags |= XCOFF_DEF_REGULAR | XCOFF_MARK;
      hsym->root.type = bfd_link_hash_defined;
      hsym->root.u.def.value = 0;
    }

  /* Garbage collect unused sections.  */
  if (bfd_link_relocatable (info) || !gc)
    {
      gc = false;
      xcoff_hash_table (info)->gc = false;

      /* We still need to call xcoff_mark, in order to set ldrel_count
	 correctly.  */
      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	{
	  asection *o;

	  for (o = sub->sections; o != NULL; o = o->next)
	    {
	      /* We shouldn't unconditionaly mark the TOC section.
		 The output file should only have a TOC if either
		 (a) one of the input files did or (b) we end up
		 creating TOC references as part of the link process.  */
	      if (o != xcoff_hash_table (info)->toc_section
		  && o->gc_mark == 0)
		{
		  if (! xcoff_mark (info, o))
		    goto error_return;
		}
	    }
	}
    }
  else
    {
      if (entry != NULL
	  && !xcoff_mark_symbol_by_name (info, entry, XCOFF_ENTRY))
	goto error_return;
      if (info->init_function != NULL
	  && !xcoff_mark_symbol_by_name (info, info->init_function, 0))
	goto error_return;
      if (info->fini_function != NULL
	  && !xcoff_mark_symbol_by_name (info, info->fini_function, 0))
	goto error_return;
      if (auto_export_flags != 0)
	{
	  xcoff_link_hash_traverse (xcoff_hash_table (info),
				    xcoff_mark_auto_exports, ldinfo);
	  if (ldinfo->failed)
	    goto error_return;
	}
      xcoff_sweep (info);
      xcoff_hash_table (info)->gc = true;
    }

  /* Return special sections to the caller.  */
  for (i = 0; i < XCOFF_NUMBER_OF_SPECIAL_SECTIONS; i++)
    {
      sec = xcoff_hash_table (info)->special_sections[i];

      if (sec != NULL
	  && gc
	  && sec->gc_mark == 0)
	sec = NULL;

      special_sections[i] = sec;
    }

  if (info->input_bfds == NULL)
    /* I'm not sure what to do in this bizarre case.  */
    return true;

  xcoff_link_hash_traverse (xcoff_hash_table (info), xcoff_post_gc_symbol,
			    (void *) ldinfo);
  if (ldinfo->failed)
    goto error_return;

  if (xcoff_hash_table (info)->loader_section
      && !xcoff_size_loader_section (ldinfo))
    goto error_return;

  return true;

 error_return:
  free (ldinfo->strings);
  return false;
}

/* Lay out the .loader section, finalizing its header and
   filling the import paths  */
static bool
xcoff_build_loader_section (struct xcoff_loader_info *ldinfo)
{
  bfd *output_bfd;
  asection *lsec;
  struct xcoff_link_hash_table *htab;
  struct internal_ldhdr *ldhdr;
  struct xcoff_import_file *fl;
  char *out;

  output_bfd = ldinfo->output_bfd;
  htab = xcoff_hash_table (ldinfo->info);
  lsec = htab->loader_section;
  ldhdr = &htab->ldhdr;

  /* We could have called xcoff_size_loader_section one more time.
     However, this function is called once all the addresses have
     been layout thus the .loader section shouldn't be changed
     anymore.  */
  BFD_ASSERT (ldhdr->l_nsyms == ldinfo->ldsym_count);
  BFD_ASSERT (ldhdr->l_nreloc == ldinfo->ldrel_count);

  /* We now know the final size of the .loader section.  Allocate
     space for it.  */
  lsec->contents = bfd_zalloc (output_bfd, lsec->size);
  if (lsec->contents == NULL)
    return false;

  /* Set up the header.  */
  bfd_xcoff_swap_ldhdr_out (output_bfd, ldhdr, lsec->contents);

  /* Set up the import file names.  */
  out = (char *) lsec->contents + ldhdr->l_impoff;
  strcpy (out, ldinfo->libpath);
  out += strlen (ldinfo->libpath) + 1;
  *out++ = '\0';
  *out++ = '\0';
  for (fl = htab->imports; fl != NULL; fl = fl->next)
    {
      const char *s;

      s = fl->path;
      while ((*out++ = *s++) != '\0')
	;
      s = fl->file;
      while ((*out++ = *s++) != '\0')
	;
      s = fl->member;
      while ((*out++ = *s++) != '\0')
	;
    }

  BFD_ASSERT ((bfd_size_type) ((bfd_byte *) out - lsec->contents) == ldhdr->l_impoff + ldhdr->l_istlen);

  /* Set up the symbol string table.  */
  if (ldinfo->string_size > 0)
    {
      memcpy (out, ldinfo->strings, ldinfo->string_size);
      free (ldinfo->strings);
      ldinfo->strings = NULL;
    }

  /* We can't set up the symbol table or the relocs yet, because we
     don't yet know the final position of the various sections.  The
     .loader symbols are written out when the corresponding normal
     symbols are written out in xcoff_link_input_bfd or
     xcoff_write_global_symbol.  The .loader relocs are written out
     when the corresponding normal relocs are handled in
     xcoff_link_input_bfd.  */

  return true;
}


/* Lay out the .loader section and allocate the space for
   the other dynamic sections of XCOFF.  */
bool
bfd_xcoff_build_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct xcoff_loader_info *ldinfo;
  struct bfd_strtab_hash *debug_strtab;
  bfd_byte *debug_contents = NULL;
  bfd *sub;
  asection *sec;

  ldinfo = &(xcoff_hash_table (info)->ldinfo);

  if (xcoff_hash_table (info)->loader_section
      && !xcoff_build_loader_section (ldinfo))
    return false;

  /* Allocate space for the magic sections.  */
  sec = xcoff_hash_table (info)->linkage_section;
  if (sec->size > 0)
    {
      sec->contents = bfd_zalloc (output_bfd, sec->size);
      if (sec->contents == NULL)
	return false;
    }
  sec = xcoff_hash_table (info)->toc_section;
  if (sec->size > 0)
    {
      sec->contents = bfd_zalloc (output_bfd, sec->size);
      if (sec->contents == NULL)
	return false;
    }
  sec = xcoff_hash_table (info)->descriptor_section;
  if (sec->size > 0)
    {
      sec->contents = bfd_zalloc (output_bfd, sec->size);
      if (sec->contents == NULL)
	return false;
    }

  /* Now that we've done garbage collection, decide which symbols to keep,
     and figure out the contents of the .debug section.  */
  debug_strtab = xcoff_hash_table (info)->debug_strtab;

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *subdeb;
      bfd_size_type symcount;
      long *debug_index;
      asection **csectpp;
      unsigned int *lineno_counts;
      struct xcoff_link_hash_entry **sym_hash;
      bfd_byte *esym, *esymend;
      bfd_size_type symesz;

      if (sub->xvec != info->output_bfd->xvec)
	continue;

      if ((sub->flags & DYNAMIC) != 0
	  && !info->static_link)
	continue;

      if (! _bfd_coff_get_external_symbols (sub))
	goto error_return;

      symcount = obj_raw_syment_count (sub);
      debug_index = bfd_zalloc (sub, symcount * sizeof (long));
      if (debug_index == NULL)
	goto error_return;
      xcoff_data (sub)->debug_indices = debug_index;

      if (info->strip == strip_all
	  || info->strip == strip_debugger
	  || info->discard == discard_all)
	/* We're stripping all debugging information, so there's no need
	   to read SUB's .debug section.  */
	subdeb = NULL;
      else
	{
	  /* Grab the contents of SUB's .debug section, if any.  */
	  subdeb = bfd_get_section_by_name (sub, ".debug");
	  if (subdeb != NULL
	      && subdeb->size != 0
	      && (subdeb->flags & SEC_HAS_CONTENTS) != 0)
	    {
	      /* We use malloc and copy the names into the debug
		 stringtab, rather than bfd_alloc, because I expect
		 that, when linking many files together, many of the
		 strings will be the same.  Storing the strings in the
		 hash table should save space in this case.  */
	      if (!bfd_malloc_and_get_section (sub, subdeb, &debug_contents))
		goto error_return;
	    }
	}

      csectpp = xcoff_data (sub)->csects;
      lineno_counts = xcoff_data (sub)->lineno_counts;
      sym_hash = obj_xcoff_sym_hashes (sub);
      symesz = bfd_coff_symesz (sub);
      esym = (bfd_byte *) obj_coff_external_syms (sub);
      esymend = esym + symcount * symesz;

      while (esym < esymend)
	{
	  struct internal_syment sym;
	  union internal_auxent aux;
	  asection *csect;
	  const char *name;
	  int keep_p;

	  bfd_coff_swap_sym_in (sub, esym, &sym);

	  /* Read in the csect information, if any.  */
	  if (CSECT_SYM_P (sym.n_sclass))
	    {
	      BFD_ASSERT (sym.n_numaux > 0);
	      bfd_coff_swap_aux_in (sub, esym + symesz * sym.n_numaux,
				    sym.n_type, sym.n_sclass,
				    sym.n_numaux - 1, sym.n_numaux, &aux);
	    }

	  /* If this symbol's name is stored in the debug section,
	     get a pointer to it.  */
	  if (debug_contents != NULL
	      && sym._n._n_n._n_zeroes == 0
	      && bfd_coff_symname_in_debug (sub, &sym))
	    name = (const char *) debug_contents + sym._n._n_n._n_offset;
	  else
	    name = NULL;

	  /* Decide whether to copy this symbol to the output file.  */
	  csect = *csectpp;
	  keep_p = xcoff_keep_symbol_p (info, sub, &sym, &aux,
					*sym_hash, csect, name);
	  if (keep_p < 0)
	    goto error_return;

	  if (!keep_p)
	    /* Use a debug_index of -2 to record that a symbol should
	       be stripped.  */
	    *debug_index = -2;
	  else
	    {
	      /* See whether we should store the symbol name in the
		 output .debug section.  */
	      if (name != NULL)
		{
		  bfd_size_type indx;

		  indx = _bfd_stringtab_add (debug_strtab, name, true, true);
		  if (indx == (bfd_size_type) -1)
		    goto error_return;
		  *debug_index = indx;
		}
	      else
		*debug_index = -1;
	      if (*sym_hash != 0)
		(*sym_hash)->flags |= XCOFF_ALLOCATED;
	      if (*lineno_counts > 0)
		csect->output_section->lineno_count += *lineno_counts;
	    }

	  esym += (sym.n_numaux + 1) * symesz;
	  csectpp += sym.n_numaux + 1;
	  sym_hash += sym.n_numaux + 1;
	  lineno_counts += sym.n_numaux + 1;
	  debug_index += sym.n_numaux + 1;
	}

      if (debug_contents)
	{
	  free (debug_contents);
	  debug_contents = NULL;

	  /* Clear the size of subdeb, so that it is not included directly
	     in the output file.  */
	  subdeb->size = 0;
	}

      if (! info->keep_memory)
	{
	  if (! _bfd_coff_free_symbols (sub))
	    goto error_return;
	}
    }

  if (info->strip != strip_all
      && xcoff_hash_table (info)->debug_section != NULL)
    xcoff_hash_table (info)->debug_section->size =
      _bfd_stringtab_size (debug_strtab);

  return true;

 error_return:
  free (debug_contents);
  return false;
}

bool
bfd_xcoff_link_generate_rtinit (bfd *abfd,
				const char *init,
				const char *fini,
				bool rtld)
{
  struct bfd_in_memory *bim;

  bim = bfd_malloc ((bfd_size_type) sizeof (* bim));
  if (bim == NULL)
    return false;

  bim->size = 0;
  bim->buffer = 0;

  abfd->link.next = 0;
  abfd->format = bfd_object;
  abfd->iostream = (void *) bim;
  abfd->flags = BFD_IN_MEMORY;
  abfd->iovec = &_bfd_memory_iovec;
  abfd->direction = write_direction;
  abfd->origin = 0;
  abfd->where = 0;

  if (! bfd_xcoff_generate_rtinit (abfd, init, fini, rtld))
    return false;

  /* need to reset to unknown or it will not be read back in correctly */
  abfd->format = bfd_unknown;
  abfd->direction = read_direction;
  abfd->where = 0;

  return true;
}


/* Linker stubs.
   The stubs will be gathered in stub csects named "@FIX'number'".
   A new csect will be created by xcoff_stub_get_csect_in_range,
   everytime a relocation cannot reach its target and its section
   is too far from the others stub csects.
   The stubs will simply be code generated inside these stub
   csects.  In order to simplify the symbol table, only the symbols
   for the stub csects are written.

   As the code is dependent of the architecture, it's defined
   in the backend.

   xcoff_stub_indirect_call:
   Used when a 24 bit branch cannot reach its destination and that
   this destination isn't a global linkage symbol.

   xcoff_stub_shared_call:
   As above but when it's a global linkage symbol.
   The main difference being that it doesn't branch to the global
   linkage symbol which will then call the shared library.  It
   directly call it saving the TOC.

   TODO: -bbigtoc option should be able to be implemented using
   this stubs.  */

/* Get the name of a csect which will contain stubs.
   It has the same pattern as AIX linker: @FIX"number".  */
static char *
xcoff_stub_csect_name (unsigned int n)
{
  char buf[8];
  size_t len;
  char *csect_name;

  /* For now, allow "only" 1000000 stub csects.  */
  if (n >= 1000000)
    {
      BFD_FAIL();
      return NULL;
    }

  sprintf (buf, "%d", n);
  len = 4 + strlen (buf) + 1;

  csect_name = bfd_malloc (len);
  if (csect_name == NULL)
    return NULL;
  sprintf (csect_name, "@FIX%d", n);

  return csect_name;
}

/* Return a stub section which can be reach with a single branch
   from SECTION.  CREATE means that creating a csect is allowed.  */
static struct xcoff_link_hash_entry *
xcoff_stub_get_csect_in_range (asection *section,
			       struct bfd_link_info *info,
			       bool create)
{
  struct xcoff_link_hash_table *htab = xcoff_hash_table (info);
  struct xcoff_link_hash_entry *csect_entry;
  struct bfd_link_hash_entry *bh = NULL;
  asection *csect;
  unsigned int it;
  char *csect_name;

  /* Search for a csect in range.  */
  for (csect = htab->params->stub_bfd->sections, it = 0;
       csect != NULL;
       csect = csect->next, it++)
    {
      /* A csect is in range if everything instructions in SECTION
	 can branch to every stubs in the stub csect.  This can
	 be simplify by saying that the first entry of each sections
	 (ie the vma of this section) can reach the last entry of the
	 stub csect (ie the vma of the csect + its size).
	 However, as the stub csect might be growing its size isn't
	 fixed.  Thus, the last entry of SECTION might not be able
	 to reach the first entry of the stub csect anymore.
	 If this case happens, the following condition will be
	 false during the next pass of bfd_xcoff_size_stubs and
	 another csect will be used.
	 This means we might create more stubs than needed.  */
      bfd_vma csect_vma, section_vma;
      bfd_vma csect_last_vma, section_last_vma;

      csect_vma = (csect->output_section->vma
		   + csect->output_offset);
      csect_last_vma = (csect->output_section->vma
			+ csect->output_offset
			+ csect->size);
      section_vma = (section->output_section->vma
		     + section->output_offset);
      section_last_vma = (section->output_section->vma
			  + section->output_offset
			  + section->size);

      if (csect_last_vma - section_vma + (1 << 25) < 2 * (1 << 25)
	  && section_last_vma - csect_vma + (1 << 25) < 2 * (1 << 25))
	break;
    }

  if (!create && csect == NULL)
    return NULL;

  csect_name = xcoff_stub_csect_name (it);
  if (!csect_name)
    return NULL;

  /* A stub csect already exists, get its entry.  */
  if (csect != NULL)
    {
      csect_entry = xcoff_link_hash_lookup (htab, csect_name, false, false, true);
      free(csect_name);
      return csect_entry;
    }

  /* Create the csect and its symbol.  */
  csect = (*htab->params->add_stub_section) (".pr", section);
  if (!csect)
    {
      free(csect_name);
      return NULL;
    }

  csect->alignment_power = 2;
  csect->gc_mark = 1;
  csect->reloc_count = 0;

  /* We need to associate a VMA to this new csect.  Otherwise,
     our "in range" algorithm won't find it for the next stub.
     And as we will be adding this stub section just after the
     SECTION, we know its address.  */
  csect->output_offset = BFD_ALIGN (section->output_offset + section->size,
				    4);

  if (!_bfd_generic_link_add_one_symbol (info, htab->params->stub_bfd,
					 csect_name, BSF_GLOBAL, csect, 0,
					 NULL, true, true, &bh))
    {
      free(csect_name);
      return NULL;
    }

  csect_entry = (struct xcoff_link_hash_entry *)bh;
  csect_entry->smclas = XMC_PR;
  csect_entry->flags = XCOFF_MARK | XCOFF_DEF_REGULAR;

  free(csect_name);
  return csect_entry;
}


/* Build a name for an entry in the stub hash table.  */
static char *
xcoff_stub_name (const struct xcoff_link_hash_entry *h,
		 const struct xcoff_link_hash_entry *hcsect)
{
  char *stub_name;
  size_t len;

  if (h)
    {
      /* The name of a stub is based on its stub csect and the
	 symbol it wants to reach.  It looks like: ".@FIX0.tramp.f".
	 When the stub targets a function, the last dot of ".tramp."
	 is removed to avoid having two dot.  */
      len = (1 + 6
	     + strlen (hcsect->root.root.string)
	     + strlen (h->root.root.string)
	     + 1);
      if (h->root.root.string[0] != '.')
	len++;

      stub_name = bfd_malloc (len);
      if (stub_name == NULL)
	return stub_name;

      if (h->root.root.string[0] == '.')
	sprintf (stub_name, ".%s.tramp%s",
		 hcsect->root.root.string,
		 h->root.root.string);
      else
	sprintf (stub_name, ".%s.tramp.%s",
		 hcsect->root.root.string,
		 h->root.root.string);
    }
  else
    {
      BFD_FAIL();
      return NULL;
    }

  return stub_name;
}

/* Look up an entry in the stub hash.  */
struct xcoff_stub_hash_entry *
bfd_xcoff_get_stub_entry (asection *section,
			  struct xcoff_link_hash_entry *h,
			  struct bfd_link_info *info)
{
  struct xcoff_link_hash_table *htab = xcoff_hash_table (info);
  struct xcoff_link_hash_entry *hcsect;
  struct xcoff_stub_hash_entry *hstub;
  char *stub_name;

  hcsect = xcoff_stub_get_csect_in_range (section, info, false);
  if (!hcsect)
    return NULL;

  stub_name = xcoff_stub_name (h, hcsect);
  if (stub_name == NULL)
    return NULL;

  hstub = xcoff_stub_hash_lookup (&htab->stub_hash_table,
				  stub_name, false, false);

  free (stub_name);
  return hstub;
}

/* Check if the symbol targeted by IREL is reachable.
   Return the type of stub needed otherwise.  */
enum xcoff_stub_type
bfd_xcoff_type_of_stub (asection *sec,
			const struct internal_reloc *irel,
			bfd_vma destination,
			struct xcoff_link_hash_entry *h)
{
  bfd_vma location, offset, max_offset;

  switch (irel->r_type)
    {
    default:
      return xcoff_stub_none;

    case R_BR:
    case R_RBR:
      location = (sec->output_section->vma
		  + sec->output_offset
		  + irel->r_vaddr
		  - sec->vma);

      max_offset = 1 << 25 ;

      offset = destination - location;

      if (offset + max_offset < 2 * max_offset)
	return xcoff_stub_none;

      /* A stub is needed.  Now, check that we can make one.  */
      if (h != NULL
	  && h->descriptor != NULL)
	{
	  /* Not sure how to handle this case. For now, skip it. */
	  if (bfd_is_abs_section (h->root.u.def.section))
	    return xcoff_stub_none;

	  if (h->smclas == XMC_GL)
	    return xcoff_stub_shared_call;
	  else
	    return xcoff_stub_indirect_call;
	}
      break;
    }

  return xcoff_stub_none;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */
static struct xcoff_stub_hash_entry *
xcoff_add_stub (const char *stub_name,
		struct xcoff_link_hash_entry *hstub_csect,
		struct xcoff_link_hash_entry *htarget,
		struct bfd_link_info *info,
		enum xcoff_stub_type stub_type)
{
  struct xcoff_link_hash_table *htab = xcoff_hash_table (info);
  struct xcoff_stub_hash_entry *hstub;
  bfd_vma stub_offset;
  asection *stub_csect;

  stub_csect = hstub_csect->root.u.def.section;
  stub_offset = stub_csect->size;

  /* Update the relocation counter and the size of
     the containing csect.  The size is needed for
     the algorithm in xcoff_stub_get_csect_in_range.  */
  switch (stub_type)
    {
    default:
      BFD_FAIL ();
      return NULL;

    case xcoff_stub_indirect_call:
      stub_csect->reloc_count++;
      stub_csect->size += bfd_xcoff_stub_indirect_call_size (info->output_bfd);
	break;

    case xcoff_stub_shared_call:
      stub_csect->reloc_count++;
      stub_csect->size += bfd_xcoff_stub_shared_call_size (info->output_bfd);
      break;
    }

  /* Create the stub entry.  */
  hstub = xcoff_stub_hash_lookup (&htab->stub_hash_table, stub_name,
				       true, true);
  if (hstub == NULL)
    return NULL;

  hstub->htarget = htarget;
  hstub->stub_offset = stub_offset;

  /* For indirect call or shared call, the relocations are against
     the target descriptor.  Its toc entry will be used.  */
  if (stub_type == xcoff_stub_indirect_call
      || stub_type == xcoff_stub_shared_call)
    {
      struct xcoff_link_hash_entry *hds = htarget->descriptor;
      asection *hds_section = hds->root.u.def.section;

      hstub->htarget = hds;

      /* If the symbol haven't been marked, its section might have
	 its size and its relocation count been deleted by xcoff_sweep.
	 Restore it.  */
      if ((hds->flags & XCOFF_MARK) == 0)
	{
	  if (hds_section->size == 0
	      && hds_section->reloc_count == 0
	      && hds_section->rawsize != 0)
	    {
	      hds_section->size = hds_section->rawsize;
	      /* Always two relocations for a XMC_DS symbol.  */
	      hds_section->reloc_count = 2;
	    }

	  /* Mark the section and the symbol.  */
	  if (!xcoff_mark (info, hds_section))
	    return NULL;
	}

      /* Add a TOC entry for the descriptor if non exists.  */
      if (hds->toc_section == NULL)
	{
	  int byte_size;

	  if (bfd_xcoff_is_xcoff64 (info->output_bfd))
	    byte_size = 8;
	  else if (bfd_xcoff_is_xcoff32 (info->output_bfd))
	    byte_size = 4;
	  else
	    return NULL;

	  /* Allocate room in the fallback TOC section.  */
	  hds->toc_section = xcoff_hash_table (info)->toc_section;
	  hds->u.toc_offset = hds->toc_section->size;
	  hds->toc_section->size += byte_size;
	  if (!xcoff_mark (info, hds->toc_section))
	    return NULL;

	  /* Update relocation counters for a static and dynamic
	     R_TOC relocation.  */
	  ++hds->toc_section->reloc_count;
	  ++htab->ldinfo.ldrel_count;

	  /* Set the index to -2 to force this symbol to
	     get written out.  */
	  hds->indx = -2;
	  hds->flags |= XCOFF_SET_TOC;
	}
    }

  return hstub;
}

static bool
xcoff_build_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct xcoff_stub_hash_entry *hstub
    = (struct xcoff_stub_hash_entry *) gen_entry;

  bfd *stub_bfd;
  bfd *output_bfd;
  struct bfd_link_info *info;
  bfd_byte *loc;
  bfd_byte *p;
  unsigned int i;

  info = (struct bfd_link_info *) in_arg;
  stub_bfd = xcoff_hash_table (info)->params->stub_bfd;
  output_bfd = info->output_bfd;

  /* Fail if the target section could not be assigned to an output
     section.  The user should fix his linker script.  */
  if (hstub->target_section != NULL
      && hstub->target_section->output_section == NULL
      && info->non_contiguous_regions)
    info->callbacks->einfo (_("%F%P: Could not assign `%pA' to an output section. "
			      "Retry without --enable-non-contiguous-regions.\n"),
			    hstub->target_section);

  loc = (hstub->hcsect->root.u.def.section->contents
	 + hstub->stub_offset);
  p = loc;

  switch (hstub->stub_type)
    {
    case xcoff_stub_indirect_call:
      BFD_ASSERT (hstub->htarget->toc_section != NULL);
      /* The first instruction in the stub code needs to be
	 cooked to hold the correct offset in the toc.  It will
	 be filled by xcoff_stub_create_relocations.  */
      for (i = 0; i < bfd_xcoff_stub_indirect_call_size(output_bfd) / 4; i++)
	bfd_put_32 (stub_bfd,
		    (bfd_vma) bfd_xcoff_stub_indirect_call_code(output_bfd, i),
		    &p[4 * i]);
      break;

    case xcoff_stub_shared_call:
      BFD_ASSERT (hstub->htarget->toc_section != NULL);
      /* The first instruction in the glink code needs to be
	 cooked to hold the correct offset in the toc.  It will
	 be filled by xcoff_stub_create_relocations.  */
      for (i = 0; i < bfd_xcoff_stub_shared_call_size(output_bfd) / 4; i++)
	bfd_put_32 (stub_bfd,
		    (bfd_vma) bfd_xcoff_stub_shared_call_code(output_bfd, i),
		    &p[4 * i]);

      break;

    default:
      BFD_FAIL ();
      return false;
    }
  return true;
}

/* Check relocations and adds stubs if needed.  */

bool
bfd_xcoff_size_stubs (struct bfd_link_info *info)
{
  struct xcoff_link_hash_table *htab = xcoff_hash_table (info);
  struct xcoff_loader_info *ldinfo = &(htab->ldinfo);

  while (1)
    {
      bfd *input_bfd;
      bool stub_changed = false;

      for (input_bfd = info->input_bfds;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next)
	{
	  asection *section;
	  bfd_size_type symcount;
	  bfd_size_type symesz;
	  bfd_byte *esyms;

	  if (bfd_get_flavour (input_bfd) != bfd_target_xcoff_flavour)
	    continue;

	  symcount = obj_raw_syment_count (input_bfd);
	  if (!symcount)
	    continue;
	  symesz = bfd_coff_symesz (input_bfd);
	  esyms = (bfd_byte *) obj_coff_external_syms (input_bfd);

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      struct internal_reloc *internal_relocs;
	      struct internal_reloc *irel, *irelend;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != info->output_bfd)
		continue;

	      /* This section have been garbage-collected.  */
	      if (section->gc_mark == 0)
		continue;

	      /* Read in the relocs.  */
	      internal_relocs = (xcoff_read_internal_relocs
				 (input_bfd, section, true, NULL,
				  false, NULL));
	      if (internal_relocs == NULL)
		goto error_ret;

	      irel = internal_relocs;
	      irelend = irel + section->reloc_count;
	      for (; irel < irelend; irel++)
		{
		  enum xcoff_stub_type stub_type;
		  struct xcoff_link_hash_entry *hsym = NULL;
		  struct xcoff_link_hash_entry *hstub_csect = NULL;
		  struct xcoff_stub_hash_entry *hstub = NULL;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  char *stub_name;

		  if (irel->r_symndx == -1)
		    continue;

		  switch (irel->r_type)
		    {
		    default:
		      continue;

		    case R_BR:
		    case R_RBR:
		      break;
		    }

		  /* Retrieve targeted symbol address */
		  hsym = obj_xcoff_sym_hashes (input_bfd)[irel->r_symndx];
		  if (hsym == NULL)
		    {
		      struct internal_syment sym;
		      if ((long unsigned int)irel->r_symndx > symcount)
			{
			  BFD_FAIL();
			  goto error_ret;
			}

		      bfd_coff_swap_sym_in (input_bfd,
					    (void *) esyms + irel->r_symndx * symesz,
					    (void *) &sym);

		      sym_sec = xcoff_data (input_bfd)->csects[irel->r_symndx];
		      sym_value = sym.n_value - sym_sec->vma;

		      destination = (sym_value
				     + sym_sec->output_section->vma
				     + sym_sec->output_offset);
		    }
		  else if (hsym->root.type == bfd_link_hash_defined
			   || hsym->root.type == bfd_link_hash_defweak)
		    {
		      sym_sec = hsym->root.u.def.section;
		      sym_value = hsym->root.u.def.value;
		      destination = (sym_value
				     + sym_sec->output_section->vma
				     + sym_sec->output_offset);
		    }
		  else
		    {
		      bfd_set_error (bfd_error_bad_value);
		      goto error_ret;
		    }

		  /* I'm not sure how to handle this case. Skip it for now.  */
		  if (bfd_is_abs_section (sym_sec))
		    continue;

		  stub_type = bfd_xcoff_type_of_stub (section, irel, destination, hsym);

		  if (stub_type == xcoff_stub_none)
		    continue;

		  /* Get a stub csect in ranch.  */
		  hstub_csect = xcoff_stub_get_csect_in_range (section, info, true);
		  if (!hstub_csect)
		    {
		      /* xgettext:c-format */
		      _bfd_error_handler (_("%pB: Unable to find a stub csect in range"
					    "of relocation at %#" PRIx64 " targeting"
					    "'%s'"),
					  section->owner, (uint64_t) irel->r_vaddr,
					  hsym->root.root.string);
		      goto error_ret;
		    }

		  /* Get the name of this stub.  */
		  stub_name = xcoff_stub_name (hsym, hstub_csect);
		  if (!stub_name)
		    goto error_ret;

		  hstub = xcoff_stub_hash_lookup (&(xcoff_hash_table (info)->stub_hash_table),
						       stub_name, false, false);

		  /* A stub entry inside the in range csect already exists.  */
		  if (hstub != NULL)
		    {
		      free (stub_name);
		      continue;
		    }

		  stub_changed = true;

		  hstub = xcoff_add_stub (stub_name, hstub_csect, hsym, info, stub_type);
		  if (hstub == NULL)
		    {
		      /* xgettext:c-format */
		      _bfd_error_handler (_("%pB: Cannot create stub entry '%s'"),
					  section->owner, stub_name);
		      free (stub_name);
		      goto error_ret;
		    }

		  hstub->stub_type = stub_type;
		  hstub->hcsect = hstub_csect;
		  hstub->target_section = sym_sec;
		  free (stub_name);
		}
	    }
	}

      if (!stub_changed)
	break;

      /* Update the size of the loader.  */
      if (xcoff_hash_table (info)->loader_section
	  && !xcoff_size_loader_section (ldinfo))
	goto error_ret;

      /* Ask the linker to do its stuff.  */
      (*htab->params->layout_sections_again) ();

    }
  return true;

 error_ret:
  bfd_set_error (bfd_error_bad_value);
  return false;
}

bool
bfd_xcoff_build_stubs (struct bfd_link_info *info)
{
  struct xcoff_link_hash_table *htab = xcoff_hash_table (info);
  asection *stub_sec;

  for (stub_sec = htab->params->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    {
      bfd_size_type size;

      /* Allocate memory to hold the linker stubs.  */
      size = stub_sec->size;
      stub_sec->contents = bfd_zalloc (htab->params->stub_bfd, size);
      if (stub_sec->contents == NULL && size != 0)
	return false;

    }

  /* Build the stubs as directed by the stub hash table.  */
  bfd_hash_traverse (&htab->stub_hash_table, xcoff_build_one_stub, info);
  return true;
}

/* Create and apply relocations made by a stub entry.  */
static bool
xcoff_stub_create_relocations (struct bfd_hash_entry *bh, void * inf)
{
  struct xcoff_stub_hash_entry *hstub
    = (struct xcoff_stub_hash_entry *) bh;
  struct xcoff_final_link_info *flinfo
    = (struct xcoff_final_link_info *) inf;

  bfd *output_bfd;
  struct internal_reloc *irel;
  struct xcoff_link_hash_entry **rel_hash;
  struct xcoff_link_hash_entry *htarget;
  asection *sec, *osec;
  bfd_vma off;
  bfd_byte *p;

  htarget = hstub->htarget;
  sec = hstub->hcsect->root.u.def.section;
  osec = sec->output_section;

  irel = (flinfo->section_info[osec->target_index].relocs
	  + osec->reloc_count);
  rel_hash = (flinfo->section_info[osec->target_index].rel_hashes
	      + osec->output_section->reloc_count);
  *rel_hash = NULL;
  output_bfd = flinfo->output_bfd;

  irel->r_symndx = htarget->indx;
  irel->r_vaddr = (osec->vma
		   + sec->output_offset
		   + hstub->hcsect->root.u.def.value
		   + hstub->stub_offset);

  p = (sec->contents
       + hstub->stub_offset);

  switch (hstub->stub_type)
    {
    default:
      BFD_FAIL ();
      return false;

      /* The first instruction of this stub code need
	 a R_TOC relocation.  */
    case xcoff_stub_indirect_call:
    case xcoff_stub_shared_call:
      irel->r_size = 0xf;
      irel->r_type = R_TOC;

      /* Retrieve the toc offset of the target which is
	 a function descriptor.  */
      BFD_ASSERT (htarget->toc_section != NULL);
      if ((htarget->flags & XCOFF_SET_TOC) != 0)
	off = hstub->htarget->u.toc_offset;
      else
	off = (htarget->toc_section->output_section->vma
	       + htarget->toc_section->output_offset
	       - xcoff_data (flinfo->output_bfd)->toc);
      if ((off & 0xffff) != off)
	{
	  _bfd_error_handler
	    (_("TOC overflow during stub generation; try -mminimal-toc "
	       "when compiling"));
	  bfd_set_error (bfd_error_file_too_big);
	  return false;
	}

      bfd_put_16 (output_bfd, off & 0xffff, p+2);
      break;
    }

  ++osec->reloc_count;
  return true;
}


/* Return the section that defines H.  Return null if no section does.  */

static asection *
xcoff_symbol_section (struct xcoff_link_hash_entry *h)
{
  switch (h->root.type)
    {
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      return h->root.u.def.section;

    case bfd_link_hash_common:
      return h->root.u.c.p->section;

    default:
      return NULL;
    }
}

/* Add a .loader relocation for input relocation IREL.  If the loader
   relocation should be against an output section, HSEC points to the
   input section that IREL is against, otherwise HSEC is null.  H is the
   symbol that IREL is against, or null if it isn't against a global symbol.
   REFERENCE_BFD is the bfd to use in error messages about the relocation.  */

static bool
xcoff_create_ldrel (bfd *output_bfd, struct xcoff_final_link_info *flinfo,
		    asection *output_section, bfd *reference_bfd,
		    struct internal_reloc *irel, asection *hsec,
		    struct xcoff_link_hash_entry *h)
{
  struct internal_ldrel ldrel;

  ldrel.l_vaddr = irel->r_vaddr;
  if (hsec != NULL)
    {
      const char *secname;

      secname = hsec->output_section->name;
      if (strcmp (secname, ".text") == 0)
	ldrel.l_symndx = 0;
      else if (strcmp (secname, ".data") == 0)
	ldrel.l_symndx = 1;
      else if (strcmp (secname, ".bss") == 0)
	ldrel.l_symndx = 2;
      else if (strcmp (secname, ".tdata") == 0)
	ldrel.l_symndx = -1;
      else if (strcmp (secname, ".tbss") == 0)
	ldrel.l_symndx = -2;
      else
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: loader reloc in unrecognized section `%s'"),
	     reference_bfd, secname);
	  bfd_set_error (bfd_error_nonrepresentable_section);
	  return false;
	}
    }
  else if (h != NULL)
    {
      if (h->ldindx < 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: `%s' in loader reloc but not loader sym"),
	     reference_bfd, h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      ldrel.l_symndx = h->ldindx;
    }
  else
    ldrel.l_symndx = -(bfd_size_type) 1;

  ldrel.l_rtype = (irel->r_size << 8) | irel->r_type;
  ldrel.l_rsecnm = output_section->target_index;
  if (xcoff_hash_table (flinfo->info)->textro
      && strcmp (output_section->name, ".text") == 0)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: loader reloc in read-only section %pA"),
	 reference_bfd, output_section);
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }
  bfd_xcoff_swap_ldrel_out (output_bfd, &ldrel, flinfo->ldrel);
  flinfo->ldrel += bfd_xcoff_ldrelsz (output_bfd);
  return true;
}

/* Link an input file into the linker output file.  This function
   handles all the sections and relocations of the input file at once.  */

static bool
xcoff_link_input_bfd (struct xcoff_final_link_info *flinfo,
		      bfd *input_bfd)
{
  bfd *output_bfd;
  const char *strings;
  bfd_size_type syment_base;
  unsigned int n_tmask;
  unsigned int n_btshft;
  bool copy, hash;
  bfd_size_type isymesz;
  bfd_size_type osymesz;
  bfd_size_type linesz;
  bfd_byte *esym;
  bfd_byte *esym_end;
  struct xcoff_link_hash_entry **sym_hash;
  struct internal_syment *isymp;
  asection **csectpp;
  unsigned int *lineno_counts;
  long *debug_index;
  long *indexp;
  unsigned long output_index;
  bfd_byte *outsym;
  unsigned int incls;
  asection *oline;
  bool keep_syms;
  asection *o;

  /* We can just skip DYNAMIC files, unless this is a static link.  */
  if ((input_bfd->flags & DYNAMIC) != 0
      && ! flinfo->info->static_link)
    return true;

  /* Move all the symbols to the output file.  */
  output_bfd = flinfo->output_bfd;
  strings = NULL;
  syment_base = obj_raw_syment_count (output_bfd);
  isymesz = bfd_coff_symesz (input_bfd);
  osymesz = bfd_coff_symesz (output_bfd);
  linesz = bfd_coff_linesz (input_bfd);
  BFD_ASSERT (linesz == bfd_coff_linesz (output_bfd));

  n_tmask = coff_data (input_bfd)->local_n_tmask;
  n_btshft = coff_data (input_bfd)->local_n_btshft;

  /* Define macros so that ISFCN, et. al., macros work correctly.  */
#define N_TMASK n_tmask
#define N_BTSHFT n_btshft

  copy = false;
  if (! flinfo->info->keep_memory)
    copy = true;
  hash = true;
  if (flinfo->info->traditional_format)
    hash = false;

  if (! _bfd_coff_get_external_symbols (input_bfd))
    return false;

  /* Make one pass over the symbols and assign indices to symbols that
     we have decided to keep.  Also use create .loader symbol information
     and update information in hash table entries.  */
  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  sym_hash = obj_xcoff_sym_hashes (input_bfd);
  csectpp = xcoff_data (input_bfd)->csects;
  debug_index = xcoff_data (input_bfd)->debug_indices;
  isymp = flinfo->internal_syms;
  indexp = flinfo->sym_indices;
  output_index = syment_base;
  while (esym < esym_end)
    {
      union internal_auxent aux;
      int smtyp = 0;
      int add;

      bfd_coff_swap_sym_in (input_bfd, (void *) esym, (void *) isymp);

      /* Read in the csect information, if any.  */
      if (CSECT_SYM_P (isymp->n_sclass))
	{
	  BFD_ASSERT (isymp->n_numaux > 0);
	  bfd_coff_swap_aux_in (input_bfd,
				(void *) (esym + isymesz * isymp->n_numaux),
				isymp->n_type, isymp->n_sclass,
				isymp->n_numaux - 1, isymp->n_numaux,
				(void *) &aux);

	  smtyp = SMTYP_SMTYP (aux.x_csect.x_smtyp);
	}

      /* If this symbol is in the .loader section, swap out the
	 .loader symbol information.  If this is an external symbol
	 reference to a defined symbol, though, then wait until we get
	 to the definition.  */
      if (EXTERN_SYM_P (isymp->n_sclass)
	  && *sym_hash != NULL
	  && (*sym_hash)->ldsym != NULL
	  && xcoff_final_definition_p (input_bfd, *sym_hash, *csectpp))
	{
	  struct xcoff_link_hash_entry *h;
	  struct internal_ldsym *ldsym;

	  h = *sym_hash;
	  ldsym = h->ldsym;
	  if (isymp->n_scnum > 0)
	    {
	      ldsym->l_scnum = (*csectpp)->output_section->target_index;
	      ldsym->l_value = (isymp->n_value
				+ (*csectpp)->output_section->vma
				+ (*csectpp)->output_offset
				- (*csectpp)->vma);
	    }
	  else
	    {
	      ldsym->l_scnum = isymp->n_scnum;
	      ldsym->l_value = isymp->n_value;
	    }

	  ldsym->l_smtype = smtyp;
	  if (((h->flags & XCOFF_DEF_REGULAR) == 0
	       && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	      || (h->flags & XCOFF_IMPORT) != 0)
	    ldsym->l_smtype |= L_IMPORT;
	  if (((h->flags & XCOFF_DEF_REGULAR) != 0
	       && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	      || (h->flags & XCOFF_EXPORT) != 0)
	    ldsym->l_smtype |= L_EXPORT;
	  if ((h->flags & XCOFF_ENTRY) != 0)
	    ldsym->l_smtype |= L_ENTRY;
	  if (isymp->n_sclass == C_AIX_WEAKEXT)
	    ldsym->l_smtype |= L_WEAK;

	  ldsym->l_smclas = aux.x_csect.x_smclas;

	  if (ldsym->l_ifile == (bfd_size_type) -1)
	    ldsym->l_ifile = 0;
	  else if (ldsym->l_ifile == 0)
	    {
	      if ((ldsym->l_smtype & L_IMPORT) == 0)
		ldsym->l_ifile = 0;
	      else
		{
		  bfd *impbfd;

		  if (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak)
		    impbfd = h->root.u.def.section->owner;
		  else if (h->root.type == bfd_link_hash_undefined
			   || h->root.type == bfd_link_hash_undefweak)
		    impbfd = h->root.u.undef.abfd;
		  else
		    impbfd = NULL;

		  if (impbfd == NULL)
		    ldsym->l_ifile = 0;
		  else
		    {
		      BFD_ASSERT (impbfd->xvec == flinfo->output_bfd->xvec);
		      ldsym->l_ifile = xcoff_data (impbfd)->import_file_id;
		    }
		}
	    }

	  ldsym->l_parm = 0;

	  BFD_ASSERT (h->ldindx >= 0);
	  bfd_xcoff_swap_ldsym_out (flinfo->output_bfd, ldsym,
				    (flinfo->ldsym
				     + ((h->ldindx - 3)
					* bfd_xcoff_ldsymsz (flinfo->output_bfd))));
	  h->ldsym = NULL;

	  /* Fill in snentry now that we know the target_index.  */
	  if ((h->flags & XCOFF_ENTRY) != 0
	      && (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak))
	    {
	      xcoff_data (output_bfd)->snentry =
		h->root.u.def.section->output_section->target_index;
	    }
	}

      add = 1 + isymp->n_numaux;

      if (*debug_index == -2)
	/* We've decided to strip this symbol.  */
	*indexp = -1;
      else
	{
	  /* Assign the next unused index to this symbol.  */
	  *indexp = output_index;

	  if (EXTERN_SYM_P (isymp->n_sclass))
	    {
	      BFD_ASSERT (*sym_hash != NULL);
	      (*sym_hash)->indx = output_index;
	    }

	  /* If this is a symbol in the TOC which we may have merged
	     (class XMC_TC), remember the symbol index of the TOC
	     symbol.  */
	  if (isymp->n_sclass == C_HIDEXT
	      && aux.x_csect.x_smclas == XMC_TC
	      && *sym_hash != NULL)
	    {
	      BFD_ASSERT (((*sym_hash)->flags & XCOFF_SET_TOC) == 0);
	      BFD_ASSERT ((*sym_hash)->toc_section != NULL);
	      (*sym_hash)->u.toc_indx = output_index;
	    }

	  output_index += add;
	}

      esym += add * isymesz;
      isymp += add;
      csectpp += add;
      sym_hash += add;
      debug_index += add;
      ++indexp;
      for (--add; add > 0; --add)
	*indexp++ = -1;
    }

  /* Now write out the symbols that we decided to keep.  */

  esym = (bfd_byte *) obj_coff_external_syms (input_bfd);
  esym_end = esym + obj_raw_syment_count (input_bfd) * isymesz;
  sym_hash = obj_xcoff_sym_hashes (input_bfd);
  isymp = flinfo->internal_syms;
  indexp = flinfo->sym_indices;
  csectpp = xcoff_data (input_bfd)->csects;
  lineno_counts = xcoff_data (input_bfd)->lineno_counts;
  debug_index = xcoff_data (input_bfd)->debug_indices;
  outsym = flinfo->outsyms;
  incls = 0;
  oline = NULL;
  while (esym < esym_end)
    {
      int add;

      add = 1 + isymp->n_numaux;

      if (*indexp < 0)
	esym += add * isymesz;
      else
	{
	  struct internal_syment isym;
	  int i;

	  /* Adjust the symbol in order to output it.  */
	  isym = *isymp;
	  if (isym._n._n_n._n_zeroes == 0
	      && isym._n._n_n._n_offset != 0)
	    {
	      /* This symbol has a long name.  Enter it in the string
		 table we are building.  If *debug_index != -1, the
		 name has already been entered in the .debug section.  */
	      if (*debug_index >= 0)
		isym._n._n_n._n_offset = *debug_index;
	      else
		{
		  const char *name;
		  bfd_size_type indx;

		  name = _bfd_coff_internal_syment_name (input_bfd, &isym, NULL);

		  if (name == NULL)
		    return false;
		  indx = _bfd_stringtab_add (flinfo->strtab, name, hash, copy);
		  if (indx == (bfd_size_type) -1)
		    return false;
		  isym._n._n_n._n_offset = STRING_SIZE_SIZE + indx;
		}
	    }

	  /* Make __rtinit C_HIDEXT rather than C_EXT.  This avoids
	     multiple definition problems when linking a shared object
	     statically.  (The native linker doesn't enter __rtinit into
	     the normal table at all, but having a local symbol can make
	     the objdump output easier to read.)  */
	  if (isym.n_sclass == C_EXT
	      && *sym_hash
	      && ((*sym_hash)->flags & XCOFF_RTINIT) != 0)
	    isym.n_sclass = C_HIDEXT;

	  /* The value of a C_FILE symbol is the symbol index of the
	     next C_FILE symbol.  The value of the last C_FILE symbol
	     is -1.  We try to get this right, below, just before we
	     write the symbols out, but in the general case we may
	     have to write the symbol out twice.  */
	  if (isym.n_sclass == C_FILE)
	    {
	      if (flinfo->last_file_index != -1
		  && flinfo->last_file.n_value != (bfd_vma) *indexp)
		{
		  /* We must correct the value of the last C_FILE entry.  */
		  flinfo->last_file.n_value = *indexp;
		  if ((bfd_size_type) flinfo->last_file_index >= syment_base)
		    {
		      /* The last C_FILE symbol is in this input file.  */
		      bfd_coff_swap_sym_out (output_bfd,
					     (void *) &flinfo->last_file,
					     (void *) (flinfo->outsyms
						    + ((flinfo->last_file_index
							- syment_base)
						       * osymesz)));
		    }
		  else
		    {
		      /* We have already written out the last C_FILE
			 symbol.  We need to write it out again.  We
			 borrow *outsym temporarily.  */
		      file_ptr pos;

		      bfd_coff_swap_sym_out (output_bfd,
					     (void *) &flinfo->last_file,
					     (void *) outsym);

		      pos = obj_sym_filepos (output_bfd);
		      pos += flinfo->last_file_index * osymesz;
		      if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
			  || (bfd_bwrite (outsym, osymesz, output_bfd)
			      != osymesz))
			return false;
		    }
		}

	      flinfo->last_file_index = *indexp;
	      flinfo->last_file = isym;
	    }

	  /* The value of a C_BINCL or C_EINCL symbol is a file offset
	     into the line numbers.  We update the symbol values when
	     we handle the line numbers.  */
	  if (isym.n_sclass == C_BINCL
	      || isym.n_sclass == C_EINCL)
	    {
	      isym.n_value = flinfo->line_filepos;
	      ++incls;
	    }
	  /* The value of a C_BSTAT symbol is the symbol table
	     index of the containing csect.  */
	  else if (isym.n_sclass == C_BSTAT)
	    {
	      bfd_vma indx;

	      indx = isym.n_value;
	      if (indx < obj_raw_syment_count (input_bfd))
		{
		  long symindx;

		  symindx = flinfo->sym_indices[indx];
		  if (symindx < 0)
		    isym.n_value = 0;
		  else
		    isym.n_value = symindx;
		}
	    }
	  else if (isym.n_sclass != C_ESTAT
		   && isym.n_sclass != C_DECL
		   && isym.n_scnum > 0)
	    {
	      isym.n_scnum = (*csectpp)->output_section->target_index;
	      isym.n_value += ((*csectpp)->output_section->vma
			       + (*csectpp)->output_offset
			       - (*csectpp)->vma);
	    }

	  /* Update visibility.  */
	  if (*sym_hash)
	    {
	      isym.n_type &= ~SYM_V_MASK;
	      isym.n_type |= (*sym_hash)->visibility;
	    }

	  /* Output the symbol.  */
	  bfd_coff_swap_sym_out (output_bfd, (void *) &isym, (void *) outsym);

	  esym += isymesz;
	  outsym += osymesz;

	  for (i = 0; i < isymp->n_numaux && esym < esym_end; i++)
	    {
	      union internal_auxent aux;

	      bfd_coff_swap_aux_in (input_bfd, (void *) esym, isymp->n_type,
				    isymp->n_sclass, i, isymp->n_numaux,
				    (void *) &aux);

	      if (isymp->n_sclass == C_FILE)
		{
		  /* This is the file name (or some comment put in by
		     the compiler).  If it is long, we must put it in
		     the string table.  */
		  if (aux.x_file.x_n.x_n.x_zeroes == 0
		      && aux.x_file.x_n.x_n.x_offset != 0)
		    {
		      const char *filename;
		      bfd_size_type indx;

		      BFD_ASSERT (aux.x_file.x_n.x_n.x_offset
				  >= STRING_SIZE_SIZE);
		      if (strings == NULL)
			{
			  strings = _bfd_coff_read_string_table (input_bfd);
			  if (strings == NULL)
			    return false;
			}
		      if ((bfd_size_type) aux.x_file.x_n.x_n.x_offset >= obj_coff_strings_len (input_bfd))
			filename = _("<corrupt>");
		      else
			filename = strings + aux.x_file.x_n.x_n.x_offset;
		      indx = _bfd_stringtab_add (flinfo->strtab, filename,
						 hash, copy);
		      if (indx == (bfd_size_type) -1)
			return false;
		      aux.x_file.x_n.x_n.x_offset = STRING_SIZE_SIZE + indx;
		    }
		}
	      else if (CSECT_SYM_P (isymp->n_sclass)
		       && i + 1 == isymp->n_numaux)
		{

		  /* We don't support type checking.  I don't know if
		     anybody does.  */
		  aux.x_csect.x_parmhash = 0;
		  /* I don't think anybody uses these fields, but we'd
		     better clobber them just in case.  */
		  aux.x_csect.x_stab = 0;
		  aux.x_csect.x_snstab = 0;

		  if (SMTYP_SMTYP (aux.x_csect.x_smtyp) == XTY_LD)
		    {
		      unsigned long indx;

		      indx = aux.x_csect.x_scnlen.u64;
		      if (indx < obj_raw_syment_count (input_bfd))
			{
			  long symindx;

			  symindx = flinfo->sym_indices[indx];
			  if (symindx < 0)
			    {
			      aux.x_csect.x_scnlen.u64 = 0;
			    }
			  else
			    {
			      aux.x_csect.x_scnlen.u64 = symindx;
			    }
			}
		    }
		}
	      else if (isymp->n_sclass != C_STAT || isymp->n_type != T_NULL)
		{
		  unsigned long indx;

		  if (ISFCN (isymp->n_type)
		      || ISTAG (isymp->n_sclass)
		      || isymp->n_sclass == C_BLOCK
		      || isymp->n_sclass == C_FCN)
		    {
		      indx = aux.x_sym.x_fcnary.x_fcn.x_endndx.u32;
		      if (indx > 0
			  && indx < obj_raw_syment_count (input_bfd))
			{
			  /* We look forward through the symbol for
			     the index of the next symbol we are going
			     to include.  I don't know if this is
			     entirely right.  */
			  while (flinfo->sym_indices[indx] < 0
				 && indx < obj_raw_syment_count (input_bfd))
			    ++indx;
			  if (indx >= obj_raw_syment_count (input_bfd))
			    indx = output_index;
			  else
			    indx = flinfo->sym_indices[indx];
			  aux.x_sym.x_fcnary.x_fcn.x_endndx.u32 = indx;

			}
		    }

		  indx = aux.x_sym.x_tagndx.u32;
		  if (indx > 0 && indx < obj_raw_syment_count (input_bfd))
		    {
		      long symindx;

		      symindx = flinfo->sym_indices[indx];
		      if (symindx < 0)
			aux.x_sym.x_tagndx.u32 = 0;
		      else
			aux.x_sym.x_tagndx.u32 = symindx;
		    }

		}

	      /* Copy over the line numbers, unless we are stripping
		 them.  We do this on a symbol by symbol basis in
		 order to more easily handle garbage collection.  */
	      if (CSECT_SYM_P (isymp->n_sclass)
		  && i == 0
		  && isymp->n_numaux > 1
		  && ISFCN (isymp->n_type)
		  && aux.x_sym.x_fcnary.x_fcn.x_lnnoptr != 0)
		{
		  if (*lineno_counts == 0)
		    aux.x_sym.x_fcnary.x_fcn.x_lnnoptr = 0;
		  else
		    {
		      asection *enclosing;
		      unsigned int enc_count;
		      bfd_signed_vma linoff;
		      struct internal_lineno lin;
		      bfd_byte *linp;
		      bfd_byte *linpend;
		      bfd_vma offset;
		      file_ptr pos;
		      bfd_size_type amt;

		      /* Read in the enclosing section's line-number
			 information, if we haven't already.  */
		      o = *csectpp;
		      enclosing = xcoff_section_data (abfd, o)->enclosing;
		      enc_count = xcoff_section_data (abfd, o)->lineno_count;
		      if (oline != enclosing)
			{
			  pos = enclosing->line_filepos;
			  amt = linesz * enc_count;
			  if (bfd_seek (input_bfd, pos, SEEK_SET) != 0
			      || (bfd_bread (flinfo->linenos, amt, input_bfd)
				  != amt))
			    return false;
			  oline = enclosing;
			}

		      /* Copy across the first entry, adjusting its
			 symbol index.  */
		      linoff = (aux.x_sym.x_fcnary.x_fcn.x_lnnoptr
				- enclosing->line_filepos);
		      linp = flinfo->linenos + linoff;
		      bfd_coff_swap_lineno_in (input_bfd, linp, &lin);
		      lin.l_addr.l_symndx = *indexp;
		      bfd_coff_swap_lineno_out (output_bfd, &lin, linp);

		      /* Copy the other entries, adjusting their addresses.  */
		      linpend = linp + *lineno_counts * linesz;
		      offset = (o->output_section->vma
				+ o->output_offset
				- o->vma);
		      for (linp += linesz; linp < linpend; linp += linesz)
			{
			  bfd_coff_swap_lineno_in (input_bfd, linp, &lin);
			  lin.l_addr.l_paddr += offset;
			  bfd_coff_swap_lineno_out (output_bfd, &lin, linp);
			}

		      /* Write out the entries we've just processed.  */
		      pos = (o->output_section->line_filepos
			     + o->output_section->lineno_count * linesz);
		      amt = linesz * *lineno_counts;
		      if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
			  || bfd_bwrite (flinfo->linenos + linoff,
					 amt, output_bfd) != amt)
			return false;
		      o->output_section->lineno_count += *lineno_counts;

		      /* Record the offset of the symbol's line numbers
			 in the output file.  */
		      aux.x_sym.x_fcnary.x_fcn.x_lnnoptr = pos;

		      if (incls > 0)
			{
			  struct internal_syment *iisp, *iispend;
			  long *iindp;
			  bfd_byte *oos;
			  bfd_vma range_start, range_end;
			  int iiadd;

			  /* Update any C_BINCL or C_EINCL symbols
			     that refer to a line number in the
			     range we just output.  */
			  iisp = flinfo->internal_syms;
			  iispend = iisp + obj_raw_syment_count (input_bfd);
			  iindp = flinfo->sym_indices;
			  oos = flinfo->outsyms;
			  range_start = enclosing->line_filepos + linoff;
			  range_end = range_start + *lineno_counts * linesz;
			  while (iisp < iispend)
			    {
			      if (*iindp >= 0
				  && (iisp->n_sclass == C_BINCL
				      || iisp->n_sclass == C_EINCL)
				  && iisp->n_value >= range_start
				  && iisp->n_value < range_end)
				{
				  struct internal_syment iis;

				  bfd_coff_swap_sym_in (output_bfd, oos, &iis);
				  iis.n_value = (iisp->n_value
						 - range_start
						 + pos);
				  bfd_coff_swap_sym_out (output_bfd,
							 &iis, oos);
				  --incls;
				}

			      iiadd = 1 + iisp->n_numaux;
			      if (*iindp >= 0)
				oos += iiadd * osymesz;
			      iisp += iiadd;
			      iindp += iiadd;
			    }
			}
		    }
		}

	      bfd_coff_swap_aux_out (output_bfd, (void *) &aux, isymp->n_type,
				     isymp->n_sclass, i, isymp->n_numaux,
				     (void *) outsym);
	      outsym += osymesz;
	      esym += isymesz;
	    }
	}

      sym_hash += add;
      indexp += add;
      isymp += add;
      csectpp += add;
      lineno_counts += add;
      debug_index += add;
    }

  /* If we swapped out a C_FILE symbol, guess that the next C_FILE
     symbol will be the first symbol in the next input file.  In the
     normal case, this will save us from writing out the C_FILE symbol
     again.  */
  if (flinfo->last_file_index != -1
      && (bfd_size_type) flinfo->last_file_index >= syment_base)
    {
      flinfo->last_file.n_value = output_index;
      bfd_coff_swap_sym_out (output_bfd, (void *) &flinfo->last_file,
			     (void *) (flinfo->outsyms
				    + ((flinfo->last_file_index - syment_base)
				       * osymesz)));
    }

  /* Write the modified symbols to the output file.  */
  if (outsym > flinfo->outsyms)
    {
      file_ptr pos = obj_sym_filepos (output_bfd) + syment_base * osymesz;
      bfd_size_type amt = outsym - flinfo->outsyms;
      if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
	  || bfd_bwrite (flinfo->outsyms, amt, output_bfd) != amt)
	return false;

      BFD_ASSERT ((obj_raw_syment_count (output_bfd)
		   + (outsym - flinfo->outsyms) / osymesz)
		  == output_index);

      obj_raw_syment_count (output_bfd) = output_index;
    }

  /* Don't let the linker relocation routines discard the symbols.  */
  keep_syms = obj_coff_keep_syms (input_bfd);
  obj_coff_keep_syms (input_bfd) = true;

  /* Relocate the contents of each section.  */
  for (o = input_bfd->sections; o != NULL; o = o->next)
    {
      bfd_byte *contents;

      if (! o->linker_mark)
	/* This section was omitted from the link.  */
	continue;

      if ((o->flags & SEC_HAS_CONTENTS) == 0
	  || o->size == 0
	  || (o->flags & SEC_IN_MEMORY) != 0)
	continue;

      /* We have set filepos correctly for the sections we created to
	 represent csects, so bfd_get_section_contents should work.  */
      if (coff_section_data (input_bfd, o) != NULL
	  && coff_section_data (input_bfd, o)->contents != NULL)
	contents = coff_section_data (input_bfd, o)->contents;
      else
	{
	  bfd_size_type sz = o->rawsize ? o->rawsize : o->size;
	  if (!bfd_get_section_contents (input_bfd, o, flinfo->contents, 0, sz))
	    goto err_out;
	  contents = flinfo->contents;
	}

      if ((o->flags & SEC_RELOC) != 0)
	{
	  int target_index;
	  struct internal_reloc *internal_relocs;
	  struct internal_reloc *irel;
	  bfd_vma offset;
	  struct internal_reloc *irelend;
	  struct xcoff_link_hash_entry **rel_hash;
	  long r_symndx;

	  /* Read in the relocs.  */
	  target_index = o->output_section->target_index;
	  internal_relocs = (xcoff_read_internal_relocs
			     (input_bfd, o, false, flinfo->external_relocs,
			      true,
			      (flinfo->section_info[target_index].relocs
			       + o->output_section->reloc_count)));
	  if (internal_relocs == NULL)
	    goto err_out;

	  /* Call processor specific code to relocate the section
	     contents.  */
	  if (! bfd_coff_relocate_section (output_bfd, flinfo->info,
					   input_bfd, o,
					   contents,
					   internal_relocs,
					   flinfo->internal_syms,
					   xcoff_data (input_bfd)->csects))
	    goto err_out;

	  offset = o->output_section->vma + o->output_offset - o->vma;
	  irel = internal_relocs;
	  irelend = irel + o->reloc_count;
	  rel_hash = (flinfo->section_info[target_index].rel_hashes
		      + o->output_section->reloc_count);
	  for (; irel < irelend; irel++, rel_hash++)
	    {
	      struct xcoff_link_hash_entry *h = NULL;

	      *rel_hash = NULL;

	      /* Adjust the reloc address and symbol index.  */

	      r_symndx = irel->r_symndx;

	      if (r_symndx == -1)
		h = NULL;
	      else
		h = obj_xcoff_sym_hashes (input_bfd)[r_symndx];

	      /* In case of a R_BR or R_RBR, change the target if
		 a stub is being called.  */
	      if (h != NULL
		  && (irel->r_type == R_BR
		      || irel->r_type == R_RBR))
		{
		  asection *sym_sec;
		  bfd_vma dest;
		  struct xcoff_stub_hash_entry *hstub = NULL;
		  enum xcoff_stub_type stub_type;

		  if (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak)
		    {
		      sym_sec = h->root.u.def.section;
		      dest = (h->root.u.def.value
			      + sym_sec->output_section->vma
			      + sym_sec->output_offset);
		    }
		  else
		    {
		      BFD_FAIL ();
		      goto err_out;
		    }

		  stub_type = bfd_xcoff_type_of_stub (o, irel, dest, h);
		  if (stub_type != xcoff_stub_none)
		    {
		      hstub = bfd_xcoff_get_stub_entry (o, h, flinfo->info);
		      if (hstub == NULL)
			goto err_out;

		      h = hstub->hcsect;
		    }

		}

	      irel->r_vaddr += offset;

	      if (r_symndx != -1 && flinfo->info->strip != strip_all)
		{

		  if (h != NULL
		      && h->smclas != XMC_TD
		      && (irel->r_type == R_TOC
			  || irel->r_type == R_GL
			  || irel->r_type == R_TCL
			  || irel->r_type == R_TRL
			  || irel->r_type == R_TRLA))
		    {
		      /* This is a TOC relative reloc with a symbol
			 attached.  The symbol should be the one which
			 this reloc is for.  We want to make this
			 reloc against the TOC address of the symbol,
			 not the symbol itself.  */
		      BFD_ASSERT (h->toc_section != NULL);
		      BFD_ASSERT ((h->flags & XCOFF_SET_TOC) == 0);
		      if (h->u.toc_indx != -1)
			irel->r_symndx = h->u.toc_indx;
		      else
			{
			  struct xcoff_toc_rel_hash *n;
			  struct xcoff_link_section_info *si;
			  size_t amt;

			  amt = sizeof (* n);
			  n = bfd_alloc (flinfo->output_bfd, amt);
			  if (n == NULL)
			    goto err_out;
			  si = flinfo->section_info + target_index;
			  n->next = si->toc_rel_hashes;
			  n->h = h;
			  n->rel = irel;
			  si->toc_rel_hashes = n;
			}
		    }
		  else if (h != NULL)
		    {
		      /* This is a global symbol.  */
		      if (h->indx >= 0)
			irel->r_symndx = h->indx;
		      else
			{
			  /* This symbol is being written at the end
			     of the file, and we do not yet know the
			     symbol index.  We save the pointer to the
			     hash table entry in the rel_hash list.
			     We set the indx field to -2 to indicate
			     that this symbol must not be stripped.  */
			  *rel_hash = h;
			  h->indx = -2;
			}
		    }
		  else
		    {
		      long indx;

		      indx = flinfo->sym_indices[r_symndx];

		      if (indx == -1)
			{
			  struct internal_syment *is;

			  /* Relocations against a TC0 TOC anchor are
			     automatically transformed to be against
			     the TOC anchor in the output file.  */
			  is = flinfo->internal_syms + r_symndx;
			  if (is->n_sclass == C_HIDEXT
			      && is->n_numaux > 0)
			    {
			      void * auxptr;
			      union internal_auxent aux;

			      auxptr = ((void *)
					(((bfd_byte *)
					  obj_coff_external_syms (input_bfd))
					 + ((r_symndx + is->n_numaux)
					    * isymesz)));
			      bfd_coff_swap_aux_in (input_bfd, auxptr,
						    is->n_type, is->n_sclass,
						    is->n_numaux - 1,
						    is->n_numaux,
						    (void *) &aux);
			      if (SMTYP_SMTYP (aux.x_csect.x_smtyp) == XTY_SD
				  && aux.x_csect.x_smclas == XMC_TC0)
				indx = flinfo->toc_symindx;
			    }
			}

		      if (indx != -1)
			irel->r_symndx = indx;
		      else
			{

			  struct internal_syment *is;

			  const char *name;
			  char buf[SYMNMLEN + 1];

			  /* This reloc is against a symbol we are
			     stripping.  It would be possible to handle
			     this case, but I don't think it's worth it.  */
			  is = flinfo->internal_syms + r_symndx;

			  if (is->n_sclass != C_DWARF)
			    {
			      name = (_bfd_coff_internal_syment_name
				      (input_bfd, is, buf));

			      if (name == NULL)
				goto err_out;

			      (*flinfo->info->callbacks->unattached_reloc)
				(flinfo->info, name,
				 input_bfd, o, irel->r_vaddr);
			    }
			}
		    }
		}

	      if ((o->flags & SEC_DEBUGGING) == 0
		  && xcoff_need_ldrel_p (flinfo->info, irel, h, o))
		{
		  asection *sec;

		  if (r_symndx == -1)
		    sec = NULL;
		  else if (h == NULL)
		    sec = xcoff_data (input_bfd)->csects[r_symndx];
		  else
		    sec = xcoff_symbol_section (h);
		  if (!xcoff_create_ldrel (output_bfd, flinfo,
					   o->output_section, input_bfd,
					   irel, sec, h))
		    goto err_out;
		}
	    }

	  o->output_section->reloc_count += o->reloc_count;
	}

      /* Write out the modified section contents.  */
      if (! bfd_set_section_contents (output_bfd, o->output_section,
				      contents, (file_ptr) o->output_offset,
				      o->size))
	goto err_out;
    }

  obj_coff_keep_syms (input_bfd) = keep_syms;

  if (! flinfo->info->keep_memory)
    {
      if (! _bfd_coff_free_symbols (input_bfd))
	return false;
    }

  return true;

 err_out:
  obj_coff_keep_syms (input_bfd) = keep_syms;
  return false;
}

#undef N_TMASK
#undef N_BTSHFT

/* Sort relocs by VMA.  This is called via qsort.  */

static int
xcoff_sort_relocs (const void * p1, const void * p2)
{
  const struct internal_reloc *r1 = (const struct internal_reloc *) p1;
  const struct internal_reloc *r2 = (const struct internal_reloc *) p2;

  if (r1->r_vaddr > r2->r_vaddr)
    return 1;
  else if (r1->r_vaddr < r2->r_vaddr)
    return -1;
  else
    return 0;
}

/* Return true if section SEC is a TOC section.  */

static inline bool
xcoff_toc_section_p (asection *sec)
{
  const char *name;

  name = sec->name;
  if (name[0] == '.' && name[1] == 't')
    {
      if (name[2] == 'c')
	{
	  if (name[3] == '0' && name[4] == 0)
	    return true;
	  if (name[3] == 0)
	    return true;
	}
      if (name[2] == 'd' && name[3] == 0)
	return true;
    }
  return false;
}

/* See if the link requires a TOC (it usually does!).  If so, find a
   good place to put the TOC anchor csect, and write out the associated
   symbol.  */

static bool
xcoff_find_tc0 (bfd *output_bfd, struct xcoff_final_link_info *flinfo)
{
  bfd_vma toc_start, toc_end, start, end, best_address;
  asection *sec;
  bfd *input_bfd;
  int section_index;
  struct internal_syment irsym;
  union internal_auxent iraux;
  file_ptr pos;
  size_t size;

  /* Set [TOC_START, TOC_END) to the range of the TOC.  Record the
     index of a csect at the beginning of the TOC.  */
  toc_start = ~(bfd_vma) 0;
  toc_end = 0;
  section_index = -1;
  for (input_bfd = flinfo->info->input_bfds;
       input_bfd != NULL;
       input_bfd = input_bfd->link.next)
    for (sec = input_bfd->sections; sec != NULL; sec = sec->next)
      if (sec->gc_mark != 0 && xcoff_toc_section_p (sec))
	{
	  start = sec->output_section->vma + sec->output_offset;
	  if (toc_start > start)
	    {
	      toc_start = start;
	      section_index = sec->output_section->target_index;
	    }

	  end = start + sec->size;
	  if (toc_end < end)
	    toc_end = end;
	}

  /* There's no need for a TC0 symbol if we don't have a TOC.  */
  if (toc_end < toc_start)
    {
      xcoff_data (output_bfd)->toc = toc_start;
      return true;
    }

  if (toc_end - toc_start < 0x8000)
    /* Every TOC csect can be accessed from TOC_START.  */
    best_address = toc_start;
  else
    {
      /* Find the lowest TOC csect that is still within range of TOC_END.  */
      best_address = toc_end;
      for (input_bfd = flinfo->info->input_bfds;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link.next)
	for (sec = input_bfd->sections; sec != NULL; sec = sec->next)
	  if (sec->gc_mark != 0 && xcoff_toc_section_p (sec))
	    {
	      start = sec->output_section->vma + sec->output_offset;
	      if (start < best_address
		  && start + 0x8000 >= toc_end)
		{
		  best_address = start;
		  section_index = sec->output_section->target_index;
		}
	    }

      /* Make sure that the start of the TOC is also within range.  */
      if (best_address > toc_start + 0x8000)
	{
	  _bfd_error_handler
	    (_("TOC overflow: %#" PRIx64 " > 0x10000; try -mminimal-toc "
	       "when compiling"),
	     (uint64_t) (toc_end - toc_start));
	  bfd_set_error (bfd_error_file_too_big);
	  return false;
	}
    }

  /* Record the chosen TOC value.  */
  flinfo->toc_symindx = obj_raw_syment_count (output_bfd);
  xcoff_data (output_bfd)->toc = best_address;
  xcoff_data (output_bfd)->sntoc = section_index;

  /* Fill out the TC0 symbol.  */
  if (!bfd_xcoff_put_symbol_name (output_bfd, flinfo->info, flinfo->strtab,
				  &irsym, "TOC"))
    return false;
  irsym.n_value = best_address;
  irsym.n_scnum = section_index;
  irsym.n_sclass = C_HIDEXT;
  irsym.n_type = T_NULL;
  irsym.n_numaux = 1;
  bfd_coff_swap_sym_out (output_bfd, &irsym, flinfo->outsyms);

  /* Fill out the auxiliary csect information.  */
  memset (&iraux, 0, sizeof iraux);
  iraux.x_csect.x_smtyp = XTY_SD;
  iraux.x_csect.x_smclas = XMC_TC0;
  iraux.x_csect.x_scnlen.u64 = 0;
  bfd_coff_swap_aux_out (output_bfd, &iraux, T_NULL, C_HIDEXT, 0, 1,
			 flinfo->outsyms + bfd_coff_symesz (output_bfd));

  /* Write the contents to the file.  */
  pos = obj_sym_filepos (output_bfd);
  pos += obj_raw_syment_count (output_bfd) * bfd_coff_symesz (output_bfd);
  size = 2 * bfd_coff_symesz (output_bfd);
  if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
      || bfd_bwrite (flinfo->outsyms, size, output_bfd) != size)
    return false;
  obj_raw_syment_count (output_bfd) += 2;

  return true;
}

/* Write out a non-XCOFF global symbol.  */

static bool
xcoff_write_global_symbol (struct bfd_hash_entry *bh, void * inf)
{
  struct xcoff_link_hash_entry *h = (struct xcoff_link_hash_entry *) bh;
  struct xcoff_final_link_info *flinfo = (struct xcoff_final_link_info *) inf;
  bfd *output_bfd;
  bfd_byte *outsym;
  struct internal_syment isym;
  union internal_auxent aux;
  bool result;
  file_ptr pos;
  bfd_size_type amt;

  output_bfd = flinfo->output_bfd;
  outsym = flinfo->outsyms;

  if (h->root.type == bfd_link_hash_warning)
    {
      h = (struct xcoff_link_hash_entry *) h->root.u.i.link;
      if (h->root.type == bfd_link_hash_new)
	return true;
    }

  /* If this symbol was garbage collected, just skip it.  */
  if (xcoff_hash_table (flinfo->info)->gc
      && (h->flags & XCOFF_MARK) == 0)
    return true;

  /* If we need a .loader section entry, write it out.  */
  if (h->ldsym != NULL)
    {
      struct internal_ldsym *ldsym;
      bfd *impbfd;

      ldsym = h->ldsym;

      if (h->root.type == bfd_link_hash_undefined
	  || h->root.type == bfd_link_hash_undefweak)
	{

	  ldsym->l_value = 0;
	  ldsym->l_scnum = N_UNDEF;
	  ldsym->l_smtype = XTY_ER;
	  impbfd = h->root.u.undef.abfd;

	}
      else if (h->root.type == bfd_link_hash_defined
	       || h->root.type == bfd_link_hash_defweak)
	{
	  asection *sec;

	  sec = h->root.u.def.section;
	  ldsym->l_value = (sec->output_section->vma
			    + sec->output_offset
			    + h->root.u.def.value);
	  ldsym->l_scnum = sec->output_section->target_index;
	  ldsym->l_smtype = XTY_SD;
	  impbfd = sec->owner;

	}
      else
	abort ();

      if (((h->flags & XCOFF_DEF_REGULAR) == 0
	   && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	  || (h->flags & XCOFF_IMPORT) != 0)
	/* Clear l_smtype
	   Import symbols are defined so the check above will make
	   the l_smtype XTY_SD.  But this is not correct, it should
	   be cleared.  */
	ldsym->l_smtype |= L_IMPORT;

      if (((h->flags & XCOFF_DEF_REGULAR) != 0
	   && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	  || (h->flags & XCOFF_EXPORT) != 0)
	ldsym->l_smtype |= L_EXPORT;

      if ((h->flags & XCOFF_ENTRY) != 0)
	ldsym->l_smtype |= L_ENTRY;

      if ((h->flags & XCOFF_RTINIT) != 0)
	ldsym->l_smtype = XTY_SD;

      ldsym->l_smclas = h->smclas;

      if (ldsym->l_smtype & L_IMPORT)
	{
	  if ((h->root.type == bfd_link_hash_defined
	       || h->root.type == bfd_link_hash_defweak)
	      && (h->root.u.def.value != 0))
	    ldsym->l_smclas = XMC_XO;

	  else if ((h->flags & (XCOFF_SYSCALL32 | XCOFF_SYSCALL64)) ==
		   (XCOFF_SYSCALL32 | XCOFF_SYSCALL64))
	    ldsym->l_smclas = XMC_SV3264;

	  else if (h->flags & XCOFF_SYSCALL32)
	    ldsym->l_smclas = XMC_SV;

	  else if (h->flags & XCOFF_SYSCALL64)
	    ldsym->l_smclas = XMC_SV64;
	}

      if (ldsym->l_ifile == -(bfd_size_type) 1)
	{
	  ldsym->l_ifile = 0;
	}
      else if (ldsym->l_ifile == 0)
	{
	  if ((ldsym->l_smtype & L_IMPORT) == 0)
	    ldsym->l_ifile = 0;
	  else if (impbfd == NULL)
	    ldsym->l_ifile = 0;
	  else
	    {
	      BFD_ASSERT (impbfd->xvec == output_bfd->xvec);
	      ldsym->l_ifile = xcoff_data (impbfd)->import_file_id;
	    }
	}

      ldsym->l_parm = 0;

      BFD_ASSERT (h->ldindx >= 0);

      bfd_xcoff_swap_ldsym_out (output_bfd, ldsym,
				(flinfo->ldsym +
				 (h->ldindx - 3)
				 * bfd_xcoff_ldsymsz(flinfo->output_bfd)));
      h->ldsym = NULL;
    }

  /* If this symbol needs global linkage code, write it out.  */
  if (h->root.type == bfd_link_hash_defined
      && (h->root.u.def.section
	  == xcoff_hash_table (flinfo->info)->linkage_section))
    {
      bfd_byte *p;
      bfd_vma tocoff;
      unsigned int i;

      p = h->root.u.def.section->contents + h->root.u.def.value;

      /* The first instruction in the global linkage code loads a
	 specific TOC element.  */
      tocoff = (h->descriptor->toc_section->output_section->vma
		+ h->descriptor->toc_section->output_offset
		- xcoff_data (output_bfd)->toc);

      if ((h->descriptor->flags & XCOFF_SET_TOC) != 0)
	tocoff += h->descriptor->u.toc_offset;

      /* The first instruction in the glink code needs to be
	 cooked to hold the correct offset in the toc.  The
	 rest are just output raw.  */
      bfd_put_32 (output_bfd,
		  bfd_xcoff_glink_code(output_bfd, 0) | (tocoff & 0xffff), p);

      /* Start with i == 1 to get past the first instruction done above
	 The /4 is because the glink code is in bytes and we are going
	 4 at a pop.  */
      for (i = 1; i < bfd_xcoff_glink_code_size(output_bfd) / 4; i++)
	bfd_put_32 (output_bfd,
		    (bfd_vma) bfd_xcoff_glink_code(output_bfd, i),
		    &p[4 * i]);
    }

  /* If we created a TOC entry for this symbol, write out the required
     relocs.  */
  if ((h->flags & XCOFF_SET_TOC) != 0)
    {
      asection *tocsec;
      asection *osec;
      int oindx;
      struct internal_reloc *irel;
      struct internal_syment irsym;
      union internal_auxent iraux;

      tocsec = h->toc_section;
      osec = tocsec->output_section;
      oindx = osec->target_index;
      irel = flinfo->section_info[oindx].relocs + osec->reloc_count;
      irel->r_vaddr = (osec->vma
		       + tocsec->output_offset
		       + h->u.toc_offset);

      if (h->indx >= 0)
	irel->r_symndx = h->indx;
      else
	{
	  h->indx = -2;
	  irel->r_symndx = obj_raw_syment_count (output_bfd);
	}

      /* Initialize the aux union here instead of closer to when it is
	 written out below because the length of the csect depends on
	 whether the output is 32 or 64 bit.  */
      memset (&iraux, 0, sizeof iraux);
      iraux.x_csect.x_smtyp = XTY_SD;
      /* iraux.x_csect.x_scnlen.u64 = 4 or 8, see below.  */
      iraux.x_csect.x_smclas = XMC_TC;

      /* 32 bit uses a 32 bit R_POS to do the relocations
	 64 bit uses a 64 bit R_POS to do the relocations

	 Also needs to change the csect size : 4 for 32 bit, 8 for 64 bit

	 Which one is determined by the backend.  */
      if (bfd_xcoff_is_xcoff64 (output_bfd))
	{
	  irel->r_size = 63;
	  iraux.x_csect.x_scnlen.u64 = 8;
	}
      else if (bfd_xcoff_is_xcoff32 (output_bfd))
	{
	  irel->r_size = 31;
	  iraux.x_csect.x_scnlen.u64 = 4;
	}
      else
	return false;

      irel->r_type = R_POS;
      flinfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      /* There are two kind of linker-created TOC entry.
	 The ones importing their symbols from outside, made for the
	 global linkage.  These symbols have XCOFF_LDREL set and only
	 requires a loader relocation on their imported symbol.
	 On the other hand, symbols without XCOFF_LDREL are TOC entries
	 of internal symbols (like function descriptors made for stubs).
	 These symbols needs a loader relocation over .data and this
	 relocation must be applied.  */

      if ((h->flags & XCOFF_LDREL) != 0
	  && h->ldindx >= 0)
	{
	  if (!xcoff_create_ldrel (output_bfd, flinfo, osec,
				   output_bfd, irel, NULL, h))
	    return false;
	}
      else
	{
	  bfd_byte *p;
	  bfd_vma val;

	  p = tocsec->contents + h->u.toc_offset;
	  val = (h->root.u.def.value
		 + h->root.u.def.section->output_section->vma
		 + h->root.u.def.section->output_offset);

	  if (bfd_xcoff_is_xcoff64 (output_bfd))
	    bfd_put_64 (output_bfd, val, p);
	  else if (bfd_xcoff_is_xcoff32 (output_bfd))
	    bfd_put_32 (output_bfd, val, p);
	  else
	    return false;

	  if (!xcoff_create_ldrel (output_bfd, flinfo, osec,
				   output_bfd, irel, h->root.u.def.section, h))
	    return false;
	}

      /* We need to emit a symbol to define a csect which holds
	 the reloc.  */
      if (flinfo->info->strip != strip_all)
	{
	  result = bfd_xcoff_put_symbol_name (output_bfd, flinfo->info,
					      flinfo->strtab,
					      &irsym, h->root.root.string);
	  if (!result)
	    return false;

	  irsym.n_value = irel->r_vaddr;
	  irsym.n_scnum = osec->target_index;
	  irsym.n_sclass = C_HIDEXT;
	  irsym.n_type = T_NULL;
	  irsym.n_numaux = 1;

	  bfd_coff_swap_sym_out (output_bfd, (void *) &irsym, (void *) outsym);
	  outsym += bfd_coff_symesz (output_bfd);

	  /* Note : iraux is initialized above.  */
	  bfd_coff_swap_aux_out (output_bfd, (void *) &iraux, T_NULL, C_HIDEXT,
				 0, 1, (void *) outsym);
	  outsym += bfd_coff_auxesz (output_bfd);

	  if (h->indx >= 0)
	    {
	      /* We aren't going to write out the symbols below, so we
		 need to write them out now.  */
	      pos = obj_sym_filepos (output_bfd);
	      pos += (obj_raw_syment_count (output_bfd)
		      * bfd_coff_symesz (output_bfd));
	      amt = outsym - flinfo->outsyms;
	      if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
		  || bfd_bwrite (flinfo->outsyms, amt, output_bfd) != amt)
		return false;
	      obj_raw_syment_count (output_bfd) +=
		(outsym - flinfo->outsyms) / bfd_coff_symesz (output_bfd);

	      outsym = flinfo->outsyms;
	    }
	}
    }

  /* If this symbol is a specially defined function descriptor, write
     it out.  The first word is the address of the function code
     itself, the second word is the address of the TOC, and the third
     word is zero.

     32 bit vs 64 bit
     The addresses for the 32 bit will take 4 bytes and the addresses
     for 64 bit will take 8 bytes.  Similar for the relocs.  This type
     of logic was also done above to create a TOC entry in
     xcoff_write_global_symbol.  */
  if ((h->flags & XCOFF_DESCRIPTOR) != 0
      && h->root.type == bfd_link_hash_defined
      && (h->root.u.def.section
	  == xcoff_hash_table (flinfo->info)->descriptor_section))
    {
      asection *sec;
      asection *osec;
      int oindx;
      bfd_byte *p;
      struct xcoff_link_hash_entry *hentry;
      asection *esec;
      struct internal_reloc *irel;
      asection *tsec;
      unsigned int reloc_size, byte_size;

      if (bfd_xcoff_is_xcoff64 (output_bfd))
	{
	  reloc_size = 63;
	  byte_size = 8;
	}
      else if (bfd_xcoff_is_xcoff32 (output_bfd))
	{
	  reloc_size = 31;
	  byte_size = 4;
	}
      else
	return false;

      sec = h->root.u.def.section;
      osec = sec->output_section;
      oindx = osec->target_index;
      p = sec->contents + h->root.u.def.value;

      hentry = h->descriptor;
      BFD_ASSERT (hentry != NULL
		  && (hentry->root.type == bfd_link_hash_defined
		      || hentry->root.type == bfd_link_hash_defweak));
      esec = hentry->root.u.def.section;

      irel = flinfo->section_info[oindx].relocs + osec->reloc_count;
      irel->r_vaddr = (osec->vma
		       + sec->output_offset
		       + h->root.u.def.value);
      irel->r_symndx = esec->output_section->target_index;
      irel->r_type = R_POS;
      irel->r_size = reloc_size;
      flinfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      if (!xcoff_create_ldrel (output_bfd, flinfo, osec,
			       output_bfd, irel, esec, NULL))
	return false;

      /* There are three items to write out,
	 the address of the code
	 the address of the toc anchor
	 the environment pointer.
	 We are ignoring the environment pointer.  So set it to zero.  */
      if (bfd_xcoff_is_xcoff64 (output_bfd))
	{
	  bfd_put_64 (output_bfd,
		      (esec->output_section->vma + esec->output_offset
		       + hentry->root.u.def.value),
		      p);
	  bfd_put_64 (output_bfd, xcoff_data (output_bfd)->toc, p + 8);
	  bfd_put_64 (output_bfd, (bfd_vma) 0, p + 16);
	}
      else
	{
	  /* 32 bit backend
	     This logic was already called above so the error case where
	     the backend is neither has already been checked.  */
	  bfd_put_32 (output_bfd,
		      (esec->output_section->vma + esec->output_offset
		       + hentry->root.u.def.value),
		      p);
	  bfd_put_32 (output_bfd, xcoff_data (output_bfd)->toc, p + 4);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, p + 8);
	}

      tsec = coff_section_from_bfd_index (output_bfd,
					  xcoff_data (output_bfd)->sntoc);

      ++irel;
      irel->r_vaddr = (osec->vma
		       + sec->output_offset
		       + h->root.u.def.value
		       + byte_size);
      irel->r_symndx = tsec->output_section->target_index;
      irel->r_type = R_POS;
      irel->r_size = reloc_size;
      flinfo->section_info[oindx].rel_hashes[osec->reloc_count] = NULL;
      ++osec->reloc_count;

      if (!xcoff_create_ldrel (output_bfd, flinfo, osec,
			       output_bfd, irel, tsec, NULL))
	return false;
    }

  if (h->indx >= 0 || flinfo->info->strip == strip_all)
    {
      BFD_ASSERT (outsym == flinfo->outsyms);
      return true;
    }

  if (h->indx != -2
      && (flinfo->info->strip == strip_all
	  || (flinfo->info->strip == strip_some
	      && bfd_hash_lookup (flinfo->info->keep_hash, h->root.root.string,
				  false, false) == NULL)))
    {
      BFD_ASSERT (outsym == flinfo->outsyms);
      return true;
    }

  if (h->indx != -2
      && (h->flags & (XCOFF_REF_REGULAR | XCOFF_DEF_REGULAR)) == 0)
    {
      BFD_ASSERT (outsym == flinfo->outsyms);
      return true;
    }

  memset (&aux, 0, sizeof aux);

  h->indx = obj_raw_syment_count (output_bfd);

  result = bfd_xcoff_put_symbol_name (output_bfd, flinfo->info, flinfo->strtab,
				      &isym, h->root.root.string);
  if (!result)
    return false;

  if (h->root.type == bfd_link_hash_undefined
      || h->root.type == bfd_link_hash_undefweak)
    {
      isym.n_value = 0;
      isym.n_scnum = N_UNDEF;
      if (h->root.type == bfd_link_hash_undefweak
	  && C_WEAKEXT == C_AIX_WEAKEXT)
	isym.n_sclass = C_WEAKEXT;
      else
	isym.n_sclass = C_EXT;
      aux.x_csect.x_smtyp = XTY_ER;
    }
  else if ((h->root.type == bfd_link_hash_defined
	    || h->root.type == bfd_link_hash_defweak)
	   && h->smclas == XMC_XO)
    {
      BFD_ASSERT (bfd_is_abs_symbol (&h->root));
      isym.n_value = h->root.u.def.value;
      isym.n_scnum = N_UNDEF;
      if (h->root.type == bfd_link_hash_defweak
	  && C_WEAKEXT == C_AIX_WEAKEXT)
	isym.n_sclass = C_WEAKEXT;
      else
	isym.n_sclass = C_EXT;
      aux.x_csect.x_smtyp = XTY_ER;
    }
  else if (h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
    {
      struct xcoff_link_size_list *l;

      isym.n_value = (h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset
		      + h->root.u.def.value);
      if (bfd_is_abs_section (h->root.u.def.section->output_section))
	isym.n_scnum = N_ABS;
      else
	isym.n_scnum = h->root.u.def.section->output_section->target_index;
      isym.n_sclass = C_HIDEXT;
      aux.x_csect.x_smtyp = XTY_SD;

      /* For stub symbols, the section already has its correct size.  */
      if (h->root.u.def.section->owner == xcoff_hash_table (flinfo->info)->params->stub_bfd)
	{
	  aux.x_csect.x_scnlen.u64 = h->root.u.def.section->size;
	}
      else if ((h->flags & XCOFF_HAS_SIZE) != 0)
	{
	  for (l = xcoff_hash_table (flinfo->info)->size_list;
	       l != NULL;
	       l = l->next)
	    {
	      if (l->h == h)
		{
		  aux.x_csect.x_scnlen.u64 = l->size;
		  break;
		}
	    }
	}
    }
  else if (h->root.type == bfd_link_hash_common)
    {
      isym.n_value = (h->root.u.c.p->section->output_section->vma
		      + h->root.u.c.p->section->output_offset);
      isym.n_scnum = h->root.u.c.p->section->output_section->target_index;
      isym.n_sclass = C_EXT;
      aux.x_csect.x_smtyp = XTY_CM;
      aux.x_csect.x_scnlen.u64 = h->root.u.c.size;
    }
  else
    abort ();

  isym.n_type = T_NULL;
  isym.n_numaux = 1;

  bfd_coff_swap_sym_out (output_bfd, (void *) &isym, (void *) outsym);
  outsym += bfd_coff_symesz (output_bfd);

  aux.x_csect.x_smclas = h->smclas;
  bfd_coff_swap_aux_out (output_bfd, (void *) &aux, T_NULL, isym.n_sclass, 0, 1,
			 (void *) outsym);
  outsym += bfd_coff_auxesz (output_bfd);

  if ((h->root.type == bfd_link_hash_defined
       || h->root.type == bfd_link_hash_defweak)
      && h->smclas != XMC_XO)
    {
      /* We just output an SD symbol.  Now output an LD symbol.  */
      h->indx += 2;

      if (h->root.type == bfd_link_hash_defweak
	  && C_WEAKEXT == C_AIX_WEAKEXT)
	isym.n_sclass = C_WEAKEXT;
      else
	isym.n_sclass = C_EXT;
      bfd_coff_swap_sym_out (output_bfd, (void *) &isym, (void *) outsym);
      outsym += bfd_coff_symesz (output_bfd);

      aux.x_csect.x_smtyp = XTY_LD;
      aux.x_csect.x_scnlen.u64 = obj_raw_syment_count (output_bfd);
      bfd_coff_swap_aux_out (output_bfd, (void *) &aux, T_NULL, C_EXT, 0, 1,
			     (void *) outsym);
      outsym += bfd_coff_auxesz (output_bfd);
    }

  pos = obj_sym_filepos (output_bfd);
  pos += obj_raw_syment_count (output_bfd) * bfd_coff_symesz (output_bfd);
  amt = outsym - flinfo->outsyms;
  if (bfd_seek (output_bfd, pos, SEEK_SET) != 0
      || bfd_bwrite (flinfo->outsyms, amt, output_bfd) != amt)
    return false;
  obj_raw_syment_count (output_bfd) +=
    (outsym - flinfo->outsyms) / bfd_coff_symesz (output_bfd);

  return true;
}

/* Handle a link order which is supposed to generate a reloc.  */

static bool
xcoff_reloc_link_order (bfd *output_bfd,
			struct xcoff_final_link_info *flinfo,
			asection *output_section,
			struct bfd_link_order *link_order)
{
  reloc_howto_type *howto;
  struct xcoff_link_hash_entry *h;
  asection *hsec;
  bfd_vma hval;
  bfd_vma addend;
  struct internal_reloc *irel;
  struct xcoff_link_hash_entry **rel_hash_ptr;

  if (link_order->type == bfd_section_reloc_link_order)
    /* We need to somehow locate a symbol in the right section.  The
       symbol must either have a value of zero, or we must adjust
       the addend by the value of the symbol.  FIXME: Write this
       when we need it.  The old linker couldn't handle this anyhow.  */
    abort ();

  howto = bfd_reloc_type_lookup (output_bfd, link_order->u.reloc.p->reloc);
  if (howto == NULL)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  h = ((struct xcoff_link_hash_entry *)
       bfd_wrapped_link_hash_lookup (output_bfd, flinfo->info,
				     link_order->u.reloc.p->u.name,
				     false, false, true));
  if (h == NULL)
    {
      (*flinfo->info->callbacks->unattached_reloc)
	(flinfo->info, link_order->u.reloc.p->u.name, NULL, NULL, (bfd_vma) 0);
      return true;
    }

  hsec = xcoff_symbol_section (h);
  if (h->root.type == bfd_link_hash_defined
      || h->root.type == bfd_link_hash_defweak)
    hval = h->root.u.def.value;
  else
    hval = 0;

  addend = link_order->u.reloc.p->addend;
  if (hsec != NULL)
    addend += (hsec->output_section->vma
	       + hsec->output_offset
	       + hval);

  if (addend != 0)
    {
      bfd_size_type size;
      bfd_byte *buf;
      bfd_reloc_status_type rstat;
      bool ok;

      size = bfd_get_reloc_size (howto);
      buf = bfd_zmalloc (size);
      if (buf == NULL && size != 0)
	return false;

      rstat = _bfd_relocate_contents (howto, output_bfd, addend, buf);
      switch (rstat)
	{
	case bfd_reloc_ok:
	  break;
	default:
	case bfd_reloc_outofrange:
	  abort ();
	case bfd_reloc_overflow:
	  (*flinfo->info->callbacks->reloc_overflow)
	    (flinfo->info, NULL, link_order->u.reloc.p->u.name,
	     howto->name, addend, NULL, NULL, (bfd_vma) 0);
	  break;
	}
      ok = bfd_set_section_contents (output_bfd, output_section, (void *) buf,
				     (file_ptr) link_order->offset, size);
      free (buf);
      if (! ok)
	return false;
    }

  /* Store the reloc information in the right place.  It will get
     swapped and written out at the end of the final_link routine.  */
  irel = (flinfo->section_info[output_section->target_index].relocs
	  + output_section->reloc_count);
  rel_hash_ptr = (flinfo->section_info[output_section->target_index].rel_hashes
		  + output_section->reloc_count);

  memset (irel, 0, sizeof (struct internal_reloc));
  *rel_hash_ptr = NULL;

  irel->r_vaddr = output_section->vma + link_order->offset;

  if (h->indx >= 0)
    irel->r_symndx = h->indx;
  else
    {
      /* Set the index to -2 to force this symbol to get written out.  */
      h->indx = -2;
      *rel_hash_ptr = h;
      irel->r_symndx = 0;
    }

  irel->r_type = howto->type;
  irel->r_size = howto->bitsize - 1;
  if (howto->complain_on_overflow == complain_overflow_signed)
    irel->r_size |= 0x80;

  ++output_section->reloc_count;

  /* Now output the reloc to the .loader section.  */
  if (xcoff_hash_table (flinfo->info)->loader_section)
    {
      if (!xcoff_create_ldrel (output_bfd, flinfo, output_section,
			       output_bfd, irel, hsec, h))
	return false;
    }

  return true;
}

/* Do the final link step.  */

bool
_bfd_xcoff_bfd_final_link (bfd *abfd, struct bfd_link_info *info)
{
  bfd_size_type symesz;
  struct xcoff_final_link_info flinfo;
  asection *o;
  struct bfd_link_order *p;
  bfd_size_type max_contents_size;
  bfd_size_type max_sym_count;
  bfd_size_type max_lineno_count;
  bfd_size_type max_reloc_count;
  bfd_size_type max_output_reloc_count;
  file_ptr rel_filepos;
  unsigned int relsz;
  file_ptr line_filepos;
  unsigned int linesz;
  bfd *sub;
  bfd_byte *external_relocs = NULL;
  char strbuf[STRING_SIZE_SIZE];
  file_ptr pos;
  bfd_size_type amt;

  if (bfd_link_pic (info))
    abfd->flags |= DYNAMIC;

  symesz = bfd_coff_symesz (abfd);

  flinfo.info = info;
  flinfo.output_bfd = abfd;
  flinfo.strtab = NULL;
  flinfo.section_info = NULL;
  flinfo.last_file_index = -1;
  flinfo.toc_symindx = -1;
  flinfo.internal_syms = NULL;
  flinfo.sym_indices = NULL;
  flinfo.outsyms = NULL;
  flinfo.linenos = NULL;
  flinfo.contents = NULL;
  flinfo.external_relocs = NULL;

  if (xcoff_hash_table (info)->loader_section)
    {
      flinfo.ldsym = (xcoff_hash_table (info)->loader_section->contents
		     + bfd_xcoff_ldhdrsz (abfd));
      flinfo.ldrel = (xcoff_hash_table (info)->loader_section->contents
		     + bfd_xcoff_ldhdrsz (abfd)
		     + (xcoff_hash_table (info)->ldhdr.l_nsyms
			* bfd_xcoff_ldsymsz (abfd)));
    }
  else
    {
      flinfo.ldsym = NULL;
      flinfo.ldrel = NULL;
    }

  xcoff_data (abfd)->coff.link_info = info;

  flinfo.strtab = _bfd_stringtab_init ();
  if (flinfo.strtab == NULL)
    goto error_return;

  /* Count the relocation entries required for the output file.
     (We've already counted the line numbers.)  Determine a few
     maximum sizes.  */
  max_contents_size = 0;
  max_lineno_count = 0;
  max_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      o->reloc_count = 0;
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order)
	    {
	      asection *sec;

	      sec = p->u.indirect.section;

	      /* Mark all sections which are to be included in the
		 link.  This will normally be every section.  We need
		 to do this so that we can identify any sections which
		 the linker has decided to not include.  */
	      sec->linker_mark = true;

	      o->reloc_count += sec->reloc_count;

	      if ((sec->flags & SEC_IN_MEMORY) == 0)
		{
		  if (sec->rawsize > max_contents_size)
		    max_contents_size = sec->rawsize;
		  if (sec->size > max_contents_size)
		    max_contents_size = sec->size;
		}
	      if (coff_section_data (sec->owner, sec) != NULL
		  && xcoff_section_data (sec->owner, sec) != NULL
		  && (xcoff_section_data (sec->owner, sec)->lineno_count
		      > max_lineno_count))
		max_lineno_count =
		  xcoff_section_data (sec->owner, sec)->lineno_count;
	      if (sec->reloc_count > max_reloc_count)
		max_reloc_count = sec->reloc_count;
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    ++o->reloc_count;
	}
    }

  /* Compute the file positions for all the sections.  */
  if (abfd->output_has_begun)
    {
      if (xcoff_hash_table (info)->file_align != 0)
	abort ();
    }
  else
    {
      bfd_vma file_align;

      file_align = xcoff_hash_table (info)->file_align;
      if (file_align != 0)
	{
	  bool saw_contents;
	  int indx;
	  file_ptr sofar;

	  /* Insert .pad sections before every section which has
	     contents and is loaded, if it is preceded by some other
	     section which has contents and is loaded.  */
	  saw_contents = true;
	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      if (strcmp (o->name, ".pad") == 0)
		saw_contents = false;
	      else if ((o->flags & SEC_HAS_CONTENTS) != 0
		       && (o->flags & SEC_LOAD) != 0)
		{
		  if (! saw_contents)
		    saw_contents = true;
		  else
		    {
		      asection *n;

		      /* Create a pad section and place it before the section
			 that needs padding.  This requires unlinking and
			 relinking the bfd's section list.  */

		      n = bfd_make_section_anyway_with_flags (abfd, ".pad",
							      SEC_HAS_CONTENTS);
		      n->alignment_power = 0;

		      bfd_section_list_remove (abfd, n);
		      bfd_section_list_insert_before (abfd, o, n);
		      saw_contents = false;
		    }
		}
	    }

	  /* Reset the section indices after inserting the new
	     sections.  */
	  if (xcoff_data (abfd)->coff.section_by_target_index)
	    htab_empty (xcoff_data (abfd)->coff.section_by_target_index);
	  indx = 0;
	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      ++indx;
	      o->target_index = indx;
	    }
	  BFD_ASSERT ((unsigned int) indx == abfd->section_count);

	  /* Work out appropriate sizes for the .pad sections to force
	     each section to land on a page boundary.  This bit of
	     code knows what compute_section_file_positions is going
	     to do.  */
	  sofar = bfd_coff_filhsz (abfd);
	  sofar += bfd_coff_aoutsz (abfd);
	  sofar += abfd->section_count * bfd_coff_scnhsz (abfd);
	  for (o = abfd->sections; o != NULL; o = o->next)
	    if ((bfd_xcoff_is_reloc_count_overflow
		 (abfd, (bfd_vma) o->reloc_count))
		|| (bfd_xcoff_is_lineno_count_overflow
		    (abfd, (bfd_vma) o->lineno_count)))
	      /* 64 does not overflow, need to check if 32 does */
	      sofar += bfd_coff_scnhsz (abfd);

	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      if (strcmp (o->name, ".pad") == 0)
		{
		  bfd_vma pageoff;

		  BFD_ASSERT (o->size == 0);
		  pageoff = sofar & (file_align - 1);
		  if (pageoff != 0)
		    {
		      o->size = file_align - pageoff;
		      sofar += file_align - pageoff;
		      o->flags |= SEC_HAS_CONTENTS;
		    }
		}
	      else
		{
		  if ((o->flags & SEC_HAS_CONTENTS) != 0)
		    sofar += BFD_ALIGN (o->size,
					1 << o->alignment_power);
		}
	    }
	}

      if (! bfd_coff_compute_section_file_positions (abfd))
	goto error_return;
    }

  /* Allocate space for the pointers we need to keep for the relocs.  */
  {
    unsigned int i;

    /* We use section_count + 1, rather than section_count, because
       the target_index fields are 1 based.  */
    amt = abfd->section_count + 1;
    amt *= sizeof (struct xcoff_link_section_info);
    flinfo.section_info = bfd_malloc (amt);
    if (flinfo.section_info == NULL)
      goto error_return;
    for (i = 0; i <= abfd->section_count; i++)
      {
	flinfo.section_info[i].relocs = NULL;
	flinfo.section_info[i].rel_hashes = NULL;
	flinfo.section_info[i].toc_rel_hashes = NULL;
      }
  }

  /* Set the file positions for the relocs.  */
  rel_filepos = obj_relocbase (abfd);
  relsz = bfd_coff_relsz (abfd);
  max_output_reloc_count = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->reloc_count == 0)
	o->rel_filepos = 0;
      else
	{
	  /* A stripped file has no relocs.  However, we still
	     allocate the buffers, so that later code doesn't have to
	     worry about whether we are stripping or not.  */
	  if (info->strip == strip_all)
	    o->rel_filepos = 0;
	  else
	    {
	      o->flags |= SEC_RELOC;
	      o->rel_filepos = rel_filepos;
	      rel_filepos += o->reloc_count * relsz;
	    }

	  /* We don't know the indices of global symbols until we have
	     written out all the local symbols.  For each section in
	     the output file, we keep an array of pointers to hash
	     table entries.  Each entry in the array corresponds to a
	     reloc.  When we find a reloc against a global symbol, we
	     set the corresponding entry in this array so that we can
	     fix up the symbol index after we have written out all the
	     local symbols.

	     Because of this problem, we also keep the relocs in
	     memory until the end of the link.  This wastes memory.
	     We could backpatch the file later, I suppose, although it
	     would be slow.  */
	  amt = o->reloc_count;
	  amt *= sizeof (struct internal_reloc);
	  flinfo.section_info[o->target_index].relocs = bfd_malloc (amt);

	  amt = o->reloc_count;
	  amt *= sizeof (struct xcoff_link_hash_entry *);
	  flinfo.section_info[o->target_index].rel_hashes = bfd_malloc (amt);

	  if (flinfo.section_info[o->target_index].relocs == NULL
	      || flinfo.section_info[o->target_index].rel_hashes == NULL)
	    goto error_return;

	  if (o->reloc_count > max_output_reloc_count)
	    max_output_reloc_count = o->reloc_count;
	}
    }

  /* We now know the size of the relocs, so we can determine the file
     positions of the line numbers.  */
  line_filepos = rel_filepos;
  flinfo.line_filepos = line_filepos;
  linesz = bfd_coff_linesz (abfd);
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->lineno_count == 0)
	o->line_filepos = 0;
      else
	{
	  o->line_filepos = line_filepos;
	  line_filepos += o->lineno_count * linesz;
	}

      /* Reset the reloc and lineno counts, so that we can use them to
	 count the number of entries we have output so far.  */
      o->reloc_count = 0;
      o->lineno_count = 0;
    }

  obj_sym_filepos (abfd) = line_filepos;

  /* Figure out the largest number of symbols in an input BFD.  Take
     the opportunity to clear the output_has_begun fields of all the
     input BFD's.  We want at least 6 symbols, since that is the
     number which xcoff_write_global_symbol may need.  */
  max_sym_count = 6;
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      bfd_size_type sz;

      sub->output_has_begun = false;
      sz = obj_raw_syment_count (sub);
      if (sz > max_sym_count)
	max_sym_count = sz;
    }

  /* Allocate some buffers used while linking.  */
  amt = max_sym_count * sizeof (struct internal_syment);
  flinfo.internal_syms = bfd_malloc (amt);

  amt = max_sym_count * sizeof (long);
  flinfo.sym_indices = bfd_malloc (amt);

  amt = (max_sym_count + 1) * symesz;
  flinfo.outsyms = bfd_malloc (amt);

  amt = max_lineno_count * bfd_coff_linesz (abfd);
  flinfo.linenos = bfd_malloc (amt);

  amt = max_contents_size;
  flinfo.contents = bfd_malloc (amt);

  amt = max_reloc_count * relsz;
  flinfo.external_relocs = bfd_malloc (amt);

  if ((flinfo.internal_syms == NULL && max_sym_count > 0)
      || (flinfo.sym_indices == NULL && max_sym_count > 0)
      || flinfo.outsyms == NULL
      || (flinfo.linenos == NULL && max_lineno_count > 0)
      || (flinfo.contents == NULL && max_contents_size > 0)
      || (flinfo.external_relocs == NULL && max_reloc_count > 0))
    goto error_return;

  obj_raw_syment_count (abfd) = 0;

  /* Find a TOC symbol, if we need one.  */
  if (!xcoff_find_tc0 (abfd, &flinfo))
    goto error_return;

  /* We now know the position of everything in the file, except that
     we don't know the size of the symbol table and therefore we don't
     know where the string table starts.  We just build the string
     table in memory as we go along.  We process all the relocations
     for a single input file at once.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  if (p->type == bfd_indirect_link_order
	      && p->u.indirect.section->owner->xvec == abfd->xvec)
	    {
	      sub = p->u.indirect.section->owner;
	      if (! sub->output_has_begun)
		{
		  if (sub == xcoff_hash_table (info)->params->stub_bfd)
		    {
		      continue;
		    }
		  else
		    {
		      if (! xcoff_link_input_bfd (&flinfo, sub))
			{
			  _bfd_error_handler
			    (_("Unable to link input file: %s"), sub->filename);
			  bfd_set_error (bfd_error_sorry);
			  goto error_return;
			}
		    }
		  sub->output_has_begun = true;
		}
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    {
	      if (! xcoff_reloc_link_order (abfd, &flinfo, o, p))
		goto error_return;
	    }
	  else
	    {
	      if (! _bfd_default_link_order (abfd, info, o, p))
		goto error_return;
	    }
	}
    }

  /* Free up the buffers used by xcoff_link_input_bfd.  */
  free (flinfo.internal_syms);
  flinfo.internal_syms = NULL;
  free (flinfo.sym_indices);
  flinfo.sym_indices = NULL;
  free (flinfo.linenos);
  flinfo.linenos = NULL;
  free (flinfo.contents);
  flinfo.contents = NULL;
  free (flinfo.external_relocs);
  flinfo.external_relocs = NULL;

  /* The value of the last C_FILE symbol is supposed to be -1.  Write
     it out again.  */
  if (flinfo.last_file_index != -1)
    {
      flinfo.last_file.n_value = -(bfd_vma) 1;
      bfd_coff_swap_sym_out (abfd, (void *) &flinfo.last_file,
			     (void *) flinfo.outsyms);
      pos = obj_sym_filepos (abfd) + flinfo.last_file_index * symesz;
      if (bfd_seek (abfd, pos, SEEK_SET) != 0
	  || bfd_bwrite (flinfo.outsyms, symesz, abfd) != symesz)
	goto error_return;
    }

  /* Write out all the global symbols which do not come from XCOFF
     input files.  */
  bfd_hash_traverse (&info->hash->table, xcoff_write_global_symbol, &flinfo);

  /* Write out the relocations created by stub entries. The symbols
     will have been already written by xcoff_write_global_symbol.  */
  bfd_hash_traverse (&xcoff_hash_table(info)->stub_hash_table,
		     xcoff_stub_create_relocations,
		     &flinfo);

  free (flinfo.outsyms);
  flinfo.outsyms = NULL;

  /* Now that we have written out all the global symbols, we know the
     symbol indices to use for relocs against them, and we can finally
     write out the relocs.  */
  amt = max_output_reloc_count * relsz;
  external_relocs = bfd_malloc (amt);
  if (external_relocs == NULL && max_output_reloc_count != 0)
    goto error_return;

  for (o = abfd->sections; o != NULL; o = o->next)
    {
      struct internal_reloc *irel;
      struct internal_reloc *irelend;
      struct xcoff_link_hash_entry **rel_hash;
      struct xcoff_toc_rel_hash *toc_rel_hash;
      bfd_byte *erel;
      bfd_size_type rel_size;

      /* A stripped file has no relocs.  */
      if (info->strip == strip_all)
	{
	  o->reloc_count = 0;
	  continue;
	}

      if (o->reloc_count == 0)
	continue;

      irel = flinfo.section_info[o->target_index].relocs;
      irelend = irel + o->reloc_count;
      rel_hash = flinfo.section_info[o->target_index].rel_hashes;
      for (; irel < irelend; irel++, rel_hash++)
	{
	  if (*rel_hash != NULL)
	    {
	      if ((*rel_hash)->indx < 0)
		{
		  (*info->callbacks->unattached_reloc)
		    (info, (*rel_hash)->root.root.string,
		     NULL, o, irel->r_vaddr);
		  (*rel_hash)->indx = 0;
		}
	      irel->r_symndx = (*rel_hash)->indx;
	    }
	}

      for (toc_rel_hash = flinfo.section_info[o->target_index].toc_rel_hashes;
	   toc_rel_hash != NULL;
	   toc_rel_hash = toc_rel_hash->next)
	{
	  if (toc_rel_hash->h->u.toc_indx < 0)
	    {
	      (*info->callbacks->unattached_reloc)
		(info, toc_rel_hash->h->root.root.string,
		 NULL, o, toc_rel_hash->rel->r_vaddr);
	      toc_rel_hash->h->u.toc_indx = 0;
	    }
	  toc_rel_hash->rel->r_symndx = toc_rel_hash->h->u.toc_indx;
	}

      /* XCOFF requires that the relocs be sorted by address.  We tend
	 to produce them in the order in which their containing csects
	 appear in the symbol table, which is not necessarily by
	 address.  So we sort them here.  There may be a better way to
	 do this.  */
      qsort ((void *) flinfo.section_info[o->target_index].relocs,
	     o->reloc_count, sizeof (struct internal_reloc),
	     xcoff_sort_relocs);

      irel = flinfo.section_info[o->target_index].relocs;
      irelend = irel + o->reloc_count;
      erel = external_relocs;
      for (; irel < irelend; irel++, rel_hash++, erel += relsz)
	bfd_coff_swap_reloc_out (abfd, (void *) irel, (void *) erel);

      rel_size = relsz * o->reloc_count;
      if (bfd_seek (abfd, o->rel_filepos, SEEK_SET) != 0
	  || bfd_bwrite ((void *) external_relocs, rel_size, abfd) != rel_size)
	goto error_return;
    }

  free (external_relocs);
  external_relocs = NULL;

  /* Free up the section information.  */
  if (flinfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  free (flinfo.section_info[i].relocs);
	  free (flinfo.section_info[i].rel_hashes);
	}
      free (flinfo.section_info);
      flinfo.section_info = NULL;
    }

  /* Write out the stub sections.  */
  for (o = xcoff_hash_table (info)->params->stub_bfd->sections;
       o != NULL; o = o->next)
    {
      if ((o->flags & SEC_HAS_CONTENTS) == 0
	  || o->size == 0)
	continue;

      if (!bfd_set_section_contents (abfd, o->output_section, o->contents,
				     (file_ptr) o->output_offset, o->size))
	goto error_return;
    }

  /* Write out the loader section contents.  */
  o = xcoff_hash_table (info)->loader_section;
  if (o != NULL
      && o->size != 0
      && o->output_section != bfd_abs_section_ptr)
    {
      BFD_ASSERT ((bfd_byte *) flinfo.ldrel
		  == (xcoff_hash_table (info)->loader_section->contents
		      + xcoff_hash_table (info)->ldhdr.l_impoff));
      if (!bfd_set_section_contents (abfd, o->output_section, o->contents,
				     (file_ptr) o->output_offset, o->size))
	goto error_return;
    }

  /* Write out the magic sections.  */
  o = xcoff_hash_table (info)->linkage_section;
  if (o != NULL
      && o->size != 0
      && o->output_section != bfd_abs_section_ptr
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     (file_ptr) o->output_offset,
				     o->size))
    goto error_return;
  o = xcoff_hash_table (info)->toc_section;
  if (o != NULL
      && o->size != 0
      && o->output_section != bfd_abs_section_ptr
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     (file_ptr) o->output_offset,
				     o->size))
    goto error_return;
  o = xcoff_hash_table (info)->descriptor_section;
  if (o != NULL
      && o->size != 0
      && o->output_section != bfd_abs_section_ptr
      && ! bfd_set_section_contents (abfd, o->output_section, o->contents,
				     (file_ptr) o->output_offset,
				     o->size))
    goto error_return;

  /* Write out the string table.  */
  pos = obj_sym_filepos (abfd) + obj_raw_syment_count (abfd) * symesz;
  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    goto error_return;
  H_PUT_32 (abfd,
	    _bfd_stringtab_size (flinfo.strtab) + STRING_SIZE_SIZE,
	    strbuf);
  amt = STRING_SIZE_SIZE;
  if (bfd_bwrite (strbuf, amt, abfd) != amt)
    goto error_return;
  if (! _bfd_stringtab_emit (abfd, flinfo.strtab))
    goto error_return;

  _bfd_stringtab_free (flinfo.strtab);

  /* Write out the debugging string table.  */
  o = xcoff_hash_table (info)->debug_section;
  if (o != NULL
      && o->size != 0
      && o->output_section != bfd_abs_section_ptr)
    {
      struct bfd_strtab_hash *debug_strtab;

      debug_strtab = xcoff_hash_table (info)->debug_strtab;
      BFD_ASSERT (o->output_section->size - o->output_offset
		  >= _bfd_stringtab_size (debug_strtab));
      pos = o->output_section->filepos + o->output_offset;
      if (bfd_seek (abfd, pos, SEEK_SET) != 0)
	goto error_return;
      if (! _bfd_stringtab_emit (abfd, debug_strtab))
	goto error_return;
    }

  /* Setting symcount to 0 will cause write_object_contents to
     not try to write out the symbols.  */
  abfd->symcount = 0;

  return true;

 error_return:
  if (flinfo.strtab != NULL)
    _bfd_stringtab_free (flinfo.strtab);

  if (flinfo.section_info != NULL)
    {
      unsigned int i;

      for (i = 0; i < abfd->section_count; i++)
	{
	  free (flinfo.section_info[i].relocs);
	  free (flinfo.section_info[i].rel_hashes);
	}
      free (flinfo.section_info);
    }

  free (flinfo.internal_syms);
  free (flinfo.sym_indices);
  free (flinfo.outsyms);
  free (flinfo.linenos);
  free (flinfo.contents);
  free (flinfo.external_relocs);
  free (external_relocs);
  return false;
}
