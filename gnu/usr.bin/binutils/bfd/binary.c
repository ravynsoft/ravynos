/* BFD back-end for binary objects.
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support, <ian@cygnus.com>

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

/* This is a BFD backend which may be used to read or write binary
   objects.  Historically, it was used as an output format for objcopy
   in order to generate raw binary data, but is now used for other
   purposes as well.

   This is very simple.  The only complication is that the real data
   will start at some address X, and in some cases we will not want to
   include X zeroes just to get to that point.  Since the start
   address is not meaningful for this object file format, we use it
   instead to indicate the number of zeroes to skip at the start of
   the file.  objcopy cooperates by specially setting the start
   address to zero by default.  */

#include "sysdep.h"
#include "bfd.h"
#include "safe-ctype.h"
#include "libbfd.h"

/* Any bfd we create by reading a binary file has three symbols:
   a start symbol, an end symbol, and an absolute length symbol.  */
#define BIN_SYMS 3

/* Create a binary object.  Invoked via bfd_set_format.  */

static bool
binary_mkobject (bfd *abfd ATTRIBUTE_UNUSED)
{
  return true;
}

/* Any file may be considered to be a binary file, provided the target
   was not defaulted.  That is, it must be explicitly specified as
   being binary.  */

static bfd_cleanup
binary_object_p (bfd *abfd)
{
  struct stat statbuf;
  asection *sec;
  flagword flags;

  if (abfd->target_defaulted)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  abfd->symcount = BIN_SYMS;

  /* Find the file size.  */
  if (bfd_stat (abfd, &statbuf) < 0)
    {
      bfd_set_error (bfd_error_system_call);
      return NULL;
    }

  /* One data section.  */
  flags = SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_HAS_CONTENTS;
  sec = bfd_make_section_with_flags (abfd, ".data", flags);
  if (sec == NULL)
    return NULL;
  sec->vma = 0;
  sec->size = statbuf.st_size;
  sec->filepos = 0;

  abfd->tdata.any = (void *) sec;

  return _bfd_no_cleanup;
}

#define binary_close_and_cleanup     _bfd_generic_close_and_cleanup
#define binary_bfd_free_cached_info  _bfd_generic_bfd_free_cached_info
#define binary_new_section_hook      _bfd_generic_new_section_hook

/* Get contents of the only section.  */

static bool
binary_get_section_contents (bfd *abfd,
			     asection *section,
			     void * location,
			     file_ptr offset,
			     bfd_size_type count)
{
  if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0
      || bfd_bread (location, count, abfd) != count)
    return false;
  return true;
}

/* Return the amount of memory needed to read the symbol table.  */

static long
binary_get_symtab_upper_bound (bfd *abfd ATTRIBUTE_UNUSED)
{
  return (BIN_SYMS + 1) * sizeof (asymbol *);
}

/* Create a symbol name based on the bfd's filename.  */

static char *
mangle_name (bfd *abfd, char *suffix)
{
  bfd_size_type size;
  char *buf;
  char *p;

  size = (strlen (bfd_get_filename (abfd))
	  + strlen (suffix)
	  + sizeof "_binary__");

  buf = (char *) bfd_alloc (abfd, size);
  if (buf == NULL)
    return "";

  sprintf (buf, "_binary_%s_%s", bfd_get_filename (abfd), suffix);

  /* Change any non-alphanumeric characters to underscores.  */
  for (p = buf; *p; p++)
    if (! ISALNUM (*p))
      *p = '_';

  return buf;
}

/* Return the symbol table.  */

static long
binary_canonicalize_symtab (bfd *abfd, asymbol **alocation)
{
  asection *sec = (asection *) abfd->tdata.any;
  asymbol *syms;
  unsigned int i;
  size_t amt = BIN_SYMS * sizeof (asymbol);

  syms = (asymbol *) bfd_alloc (abfd, amt);
  if (syms == NULL)
    return -1;

  /* Start symbol.  */
  syms[0].the_bfd = abfd;
  syms[0].name = mangle_name (abfd, "start");
  syms[0].value = 0;
  syms[0].flags = BSF_GLOBAL;
  syms[0].section = sec;
  syms[0].udata.p = NULL;

  /* End symbol.  */
  syms[1].the_bfd = abfd;
  syms[1].name = mangle_name (abfd, "end");
  syms[1].value = sec->size;
  syms[1].flags = BSF_GLOBAL;
  syms[1].section = sec;
  syms[1].udata.p = NULL;

  /* Size symbol.  */
  syms[2].the_bfd = abfd;
  syms[2].name = mangle_name (abfd, "size");
  syms[2].value = sec->size;
  syms[2].flags = BSF_GLOBAL;
  syms[2].section = bfd_abs_section_ptr;
  syms[2].udata.p = NULL;

  for (i = 0; i < BIN_SYMS; i++)
    *alocation++ = syms++;
  *alocation = NULL;

  return BIN_SYMS;
}

#define binary_make_empty_symbol  _bfd_generic_make_empty_symbol
#define binary_print_symbol       _bfd_nosymbols_print_symbol
#define binary_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string

/* Get information about a symbol.  */

static void
binary_get_symbol_info (bfd *ignore_abfd ATTRIBUTE_UNUSED,
			asymbol *symbol,
			symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

#define binary_bfd_is_local_label_name	    bfd_generic_is_local_label_name
#define binary_get_lineno		   _bfd_nosymbols_get_lineno
#define binary_find_nearest_line	   _bfd_nosymbols_find_nearest_line
#define binary_find_nearest_line_with_alt  _bfd_nosymbols_find_nearest_line_with_alt
#define binary_find_line		   _bfd_nosymbols_find_line
#define binary_find_inliner_info	   _bfd_nosymbols_find_inliner_info
#define binary_bfd_make_debug_symbol	   _bfd_nosymbols_bfd_make_debug_symbol
#define binary_read_minisymbols		   _bfd_generic_read_minisymbols
#define binary_minisymbol_to_symbol	   _bfd_generic_minisymbol_to_symbol
#define binary_bfd_is_target_special_symbol _bfd_bool_bfd_asymbol_false

/* Set the architecture of a binary file.  */
#define binary_set_arch_mach _bfd_generic_set_arch_mach

/* Write section contents of a binary file.  */

static bool
binary_set_section_contents (bfd *abfd,
			     asection *sec,
			     const void * data,
			     file_ptr offset,
			     bfd_size_type size)
{
  if (size == 0)
    return true;

  if (! abfd->output_has_begun)
    {
      bool found_low;
      bfd_vma low;
      asection *s;

      /* The lowest section LMA sets the virtual address of the start
	 of the file.  We use this to set the file position of all the
	 sections.  */
      found_low = false;
      low = 0;
      for (s = abfd->sections; s != NULL; s = s->next)
	if (((s->flags
	      & (SEC_HAS_CONTENTS | SEC_LOAD | SEC_ALLOC | SEC_NEVER_LOAD))
	     == (SEC_HAS_CONTENTS | SEC_LOAD | SEC_ALLOC))
	    && (s->size > 0)
	    && (! found_low || s->lma < low))
	  {
	    low = s->lma;
	    found_low = true;
	  }

      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  unsigned int opb = bfd_octets_per_byte (abfd, s);

	  s->filepos = (s->lma - low) * opb;

	  /* Skip following warning check for sections that will not
	     occupy file space.  */
	  if ((s->flags
	       & (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_NEVER_LOAD))
	      != (SEC_HAS_CONTENTS | SEC_ALLOC)
	      || (s->size == 0))
	    continue;

	  /* If attempting to generate a binary file from a bfd with
	     LMA's all over the place, huge (sparse?) binary files may
	     result.  This condition attempts to detect this situation
	     and print a warning.  Better heuristics would be nice to
	     have.  */

	  if (s->filepos < 0)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("warning: writing section `%pA' at huge (ie negative) "
		 "file offset"),
	       s);
	}

      abfd->output_has_begun = true;
    }

  /* We don't want to output anything for a section that is neither
     loaded nor allocated.  The contents of such a section are not
     meaningful in the binary format.  */
  if ((sec->flags & (SEC_LOAD | SEC_ALLOC)) == 0)
    return true;
  if ((sec->flags & SEC_NEVER_LOAD) != 0)
    return true;

  return _bfd_generic_set_section_contents (abfd, sec, data, offset, size);
}

/* No space is required for header information.  */

static int
binary_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

#define binary_bfd_get_relocated_section_contents  bfd_generic_get_relocated_section_contents
#define binary_bfd_relax_section		   bfd_generic_relax_section
#define binary_bfd_gc_sections			   bfd_generic_gc_sections
#define binary_bfd_lookup_section_flags		   bfd_generic_lookup_section_flags
#define binary_bfd_merge_sections		   bfd_generic_merge_sections
#define binary_bfd_is_group_section		   bfd_generic_is_group_section
#define binary_bfd_group_name			   bfd_generic_group_name
#define binary_bfd_discard_group		   bfd_generic_discard_group
#define binary_section_already_linked		  _bfd_generic_section_already_linked
#define binary_bfd_define_common_symbol		   bfd_generic_define_common_symbol
#define binary_bfd_link_hide_symbol		   _bfd_generic_link_hide_symbol
#define binary_bfd_define_start_stop		   bfd_generic_define_start_stop
#define binary_bfd_link_hash_table_create	  _bfd_generic_link_hash_table_create
#define binary_bfd_link_just_syms		  _bfd_generic_link_just_syms
#define binary_bfd_copy_link_hash_symbol_type	  _bfd_generic_copy_link_hash_symbol_type
#define binary_bfd_link_add_symbols		  _bfd_generic_link_add_symbols
#define binary_bfd_final_link			  _bfd_generic_final_link
#define binary_bfd_link_split_section		  _bfd_generic_link_split_section
#define binary_get_section_contents_in_window	  _bfd_generic_get_section_contents_in_window
#define binary_bfd_link_check_relocs		  _bfd_generic_link_check_relocs

const bfd_target binary_vec =
{
  "binary",			/* name */
  bfd_target_unknown_flavour,	/* flavour */
  BFD_ENDIAN_UNKNOWN,		/* byteorder */
  BFD_ENDIAN_UNKNOWN,		/* header_byteorder */
  EXEC_P,			/* object_flags */
  (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE | SEC_DATA
   | SEC_ROM | SEC_HAS_CONTENTS), /* section_flags */
  0,				/* symbol_leading_char */
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
  255,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* data */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* hdrs */
  {				/* bfd_check_format */
    _bfd_dummy_target,
    binary_object_p,
    _bfd_dummy_target,
    _bfd_dummy_target,
  },
  {				/* bfd_set_format */
    _bfd_bool_bfd_false_error,
    binary_mkobject,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },
  {				/* bfd_write_contents */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_true,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },

  BFD_JUMP_TABLE_GENERIC (binary),
  BFD_JUMP_TABLE_COPY (_bfd_generic),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (binary),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (binary),
  BFD_JUMP_TABLE_LINK (binary),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};
