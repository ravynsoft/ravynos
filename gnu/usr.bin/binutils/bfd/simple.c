/* simple.c -- BFD simple client routines
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   Contributed by MontaVista Software, Inc.

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
#include "bfdlink.h"
#include "genlink.h"

static void
simple_dummy_add_to_set (struct bfd_link_info * info ATTRIBUTE_UNUSED,
			 struct bfd_link_hash_entry *entry ATTRIBUTE_UNUSED,
			 bfd_reloc_code_real_type reloc ATTRIBUTE_UNUSED,
			 bfd *abfd ATTRIBUTE_UNUSED,
			 asection *sec ATTRIBUTE_UNUSED,
			 bfd_vma value ATTRIBUTE_UNUSED)
{
}

static  void
simple_dummy_constructor (struct bfd_link_info * info ATTRIBUTE_UNUSED,
			  bool constructor ATTRIBUTE_UNUSED,
			  const char *name ATTRIBUTE_UNUSED,
			  bfd *abfd ATTRIBUTE_UNUSED,
			  asection *sec ATTRIBUTE_UNUSED,
			  bfd_vma value ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_multiple_common (struct bfd_link_info * info ATTRIBUTE_UNUSED,
			      struct bfd_link_hash_entry * entry ATTRIBUTE_UNUSED,
			      bfd * abfd ATTRIBUTE_UNUSED,
			      enum bfd_link_hash_type type ATTRIBUTE_UNUSED,
			      bfd_vma size ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_warning (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
		      const char *warning ATTRIBUTE_UNUSED,
		      const char *symbol ATTRIBUTE_UNUSED,
		      bfd *abfd ATTRIBUTE_UNUSED,
		      asection *section ATTRIBUTE_UNUSED,
		      bfd_vma address ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_undefined_symbol (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			       const char *name ATTRIBUTE_UNUSED,
			       bfd *abfd ATTRIBUTE_UNUSED,
			       asection *section ATTRIBUTE_UNUSED,
			       bfd_vma address ATTRIBUTE_UNUSED,
			       bool fatal ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_reloc_overflow (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			     struct bfd_link_hash_entry *entry ATTRIBUTE_UNUSED,
			     const char *name ATTRIBUTE_UNUSED,
			     const char *reloc_name ATTRIBUTE_UNUSED,
			     bfd_vma addend ATTRIBUTE_UNUSED,
			     bfd *abfd ATTRIBUTE_UNUSED,
			     asection *section ATTRIBUTE_UNUSED,
			     bfd_vma address ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_reloc_dangerous (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			      const char *message ATTRIBUTE_UNUSED,
			      bfd *abfd ATTRIBUTE_UNUSED,
			      asection *section ATTRIBUTE_UNUSED,
			      bfd_vma address ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_unattached_reloc (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			       const char *name ATTRIBUTE_UNUSED,
			       bfd *abfd ATTRIBUTE_UNUSED,
			       asection *section ATTRIBUTE_UNUSED,
			       bfd_vma address ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_multiple_definition (struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
				  struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED,
				  bfd *nbfd ATTRIBUTE_UNUSED,
				  asection *nsec ATTRIBUTE_UNUSED,
				  bfd_vma nval ATTRIBUTE_UNUSED)
{
}

static void
simple_dummy_einfo (const char *fmt ATTRIBUTE_UNUSED, ...)
{
}

struct saved_output_info
{
  bfd_vma offset;
  asection *section;
};

struct saved_offsets
{
  unsigned int section_count;
  struct saved_output_info *sections;
};

/* The sections in ABFD may already have output sections and offsets
   set if we are here during linking.

   DWARF-2 specifies offsets into debug sections in many cases and
   bfd_simple_get_relocated_section_contents is called to relocate
   debug info for a single relocatable object file.  So we want
   offsets relative to that object file's sections, not offsets in the
   output file.  For that reason, reset a debug section->output_offset
   to zero.

   If not called during linking then set section->output_section to
   point back to the input section, because output_section must not be
   NULL when calling the relocation routines.

   Save the original output offset and section to restore later.  */

static void
simple_save_output_info (bfd *abfd ATTRIBUTE_UNUSED,
			 asection *section,
			 void *ptr)
{
  struct saved_offsets *saved_offsets = (struct saved_offsets *) ptr;
  struct saved_output_info *output_info;

  output_info = &saved_offsets->sections[section->index];
  output_info->offset = section->output_offset;
  output_info->section = section->output_section;
  if ((section->flags & SEC_DEBUGGING) != 0
      || section->output_section == NULL)
    {
      section->output_offset = 0;
      section->output_section = section;
    }
}

static void
simple_restore_output_info (bfd *abfd ATTRIBUTE_UNUSED,
			    asection *section,
			    void *ptr)
{
  struct saved_offsets *saved_offsets = (struct saved_offsets *) ptr;
  struct saved_output_info *output_info;

  if (section->index >= saved_offsets->section_count)
    return;

  output_info = &saved_offsets->sections[section->index];
  section->output_offset = output_info->offset;
  section->output_section = output_info->section;
}

/*
FUNCTION
	bfd_simple_relocate_secton

SYNOPSIS
	bfd_byte *bfd_simple_get_relocated_section_contents
	  (bfd *abfd, asection *sec, bfd_byte *outbuf, asymbol **symbol_table);

DESCRIPTION
	Returns the relocated contents of section @var{sec}.  The symbols in
	@var{symbol_table} will be used, or the symbols from @var{abfd} if
	@var{symbol_table} is NULL.  The output offsets for debug sections will
	be temporarily reset to 0.  The result will be stored at @var{outbuf}
	or allocated with @code{bfd_malloc} if @var{outbuf} is @code{NULL}.

	Returns @code{NULL} on a fatal error; ignores errors applying
	particular relocations.
*/

bfd_byte *
bfd_simple_get_relocated_section_contents (bfd *abfd,
					   asection *sec,
					   bfd_byte *outbuf,
					   asymbol **symbol_table)
{
  struct bfd_link_info link_info;
  struct bfd_link_order link_order;
  struct bfd_link_callbacks callbacks;
  bfd_byte *contents;
  struct saved_offsets saved_offsets;
  bfd *link_next;

  /* Don't apply relocation on executable and shared library.  See
     PR 4756.  */
  if ((abfd->flags & (HAS_RELOC | EXEC_P | DYNAMIC)) != HAS_RELOC
      || ! (sec->flags & SEC_RELOC))
    {
      if (!bfd_get_full_section_contents (abfd, sec, &outbuf))
	return NULL;
      return outbuf;
    }

  /* In order to use bfd_get_relocated_section_contents, we need
     to forge some data structures that it expects.  */

  /* Fill in the bare minimum number of fields for our purposes.  */
  memset (&link_info, 0, sizeof (link_info));
  link_info.output_bfd = abfd;
  link_info.input_bfds = abfd;
  link_info.input_bfds_tail = &abfd->link.next;

  link_next = abfd->link.next;
  abfd->link.next = NULL;
  link_info.hash = _bfd_generic_link_hash_table_create (abfd);
  link_info.callbacks = &callbacks;
  /* Make sure that any fields not initialised below do not
     result in a potential indirection via a random address.  */
  memset (&callbacks, 0, sizeof callbacks);
  callbacks.warning = simple_dummy_warning;
  callbacks.undefined_symbol = simple_dummy_undefined_symbol;
  callbacks.reloc_overflow = simple_dummy_reloc_overflow;
  callbacks.reloc_dangerous = simple_dummy_reloc_dangerous;
  callbacks.unattached_reloc = simple_dummy_unattached_reloc;
  callbacks.multiple_definition = simple_dummy_multiple_definition;
  callbacks.einfo = simple_dummy_einfo;
  callbacks.multiple_common = simple_dummy_multiple_common;
  callbacks.constructor = simple_dummy_constructor;
  callbacks.add_to_set = simple_dummy_add_to_set;

  memset (&link_order, 0, sizeof (link_order));
  link_order.next = NULL;
  link_order.type = bfd_indirect_link_order;
  link_order.offset = 0;
  link_order.size = sec->size;
  link_order.u.indirect.section = sec;

  contents = NULL;

  saved_offsets.section_count = abfd->section_count;
  saved_offsets.sections = malloc (sizeof (*saved_offsets.sections)
				   * saved_offsets.section_count);
  if (saved_offsets.sections == NULL)
    goto out1;
  bfd_map_over_sections (abfd, simple_save_output_info, &saved_offsets);

  if (symbol_table == NULL)
    {
      if (!bfd_generic_link_read_symbols (abfd))
	goto out2;
      symbol_table = _bfd_generic_link_get_symbols (abfd);
    }

  contents = bfd_get_relocated_section_contents (abfd,
						 &link_info,
						 &link_order,
						 outbuf,
						 false,
						 symbol_table);
 out2:
  bfd_map_over_sections (abfd, simple_restore_output_info, &saved_offsets);
  free (saved_offsets.sections);
 out1:
  _bfd_generic_link_hash_table_free (abfd);
  abfd->link.next = link_next;
  return contents;
}
