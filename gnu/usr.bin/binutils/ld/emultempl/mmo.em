# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2001-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# This file is sourced from generic.em.

fragment <<EOF
/* Need to have this macro defined before mmix-elfnmmo, which uses the
   name for the before_allocation function, defined in ldemul.c (for
   the mmo "emulation") or in elf.em (for the elf64mmix
   "emulation").  */
#define gldmmo_before_allocation before_allocation_default

/* We include this header *not* because we expect to handle ELF here
   but because we use the map_segments function.  But this is only to
   get a weird testcase right; ld-mmix/bpo-22, forcing ELF to be
   output from the mmo emulation: -m mmo --oformat elf64-mmix!  */
#include "ldelfgen.h"

static void gld${EMULATION_NAME}_after_allocation (void);
EOF

source_em ${srcdir}/emultempl/elf-generic.em
source_em ${srcdir}/emultempl/mmix-elfnmmo.em

fragment <<EOF

/* Place an orphan section.  We use this to put random SEC_CODE or
   SEC_READONLY sections right after MMO_TEXT_SECTION_NAME.  Much borrowed
   from elf.em.  */

static lang_output_section_statement_type *
mmo_place_orphan (asection *s,
		  const char *secname,
		  int constraint ATTRIBUTE_UNUSED)
{
  static struct
  {
    flagword nonzero_flags;
    struct orphan_save orphansave;
  } holds[] =
      {
	{
	  SEC_CODE | SEC_READONLY,
	  {
	    MMO_TEXT_SECTION_NAME,
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE,
	    0, 0, 0, 0
	  }
	},
	{
	  SEC_LOAD | SEC_DATA,
	  {
	    MMO_DATA_SECTION_NAME,
	    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA,
	    0, 0, 0, 0
	  }
	},
	{
	  SEC_ALLOC,
	  {
	    ".bss",
	    SEC_ALLOC,
	    0, 0, 0, 0
	  }
	}
      };

  struct orphan_save *place = NULL;
  lang_output_section_statement_type *after;
  lang_output_section_statement_type *os;
  size_t i;
  flagword flags;
  asection *nexts;

  /* We have nothing to say for anything other than a final link or
     for sections that are excluded.  */
  if (bfd_link_relocatable (&link_info)
      || (s->flags & SEC_EXCLUDE) != 0)
    return NULL;

  os = lang_output_section_find (secname);

  /* We have an output section by this name.  Place the section inside it
     (regardless of whether the linker script lists it as input).  */
  if (os != NULL)
    {
      lang_add_section (&os->children, s, NULL, NULL, os);
      return os;
    }

  flags = s->flags;
  if (!bfd_link_relocatable (&link_info))
    {
      nexts = s;
      while ((nexts = bfd_get_next_section_by_name (nexts->owner, nexts))
	     != NULL)
	if (nexts->output_section == NULL
	    && (nexts->flags & SEC_EXCLUDE) == 0
	    && ((nexts->flags ^ flags) & (SEC_LOAD | SEC_ALLOC)) == 0
	    && (nexts->owner->flags & DYNAMIC) == 0
	    && !bfd_input_just_syms (nexts->owner))
	  flags = (((flags ^ SEC_READONLY) | (nexts->flags ^ SEC_READONLY))
		   ^ SEC_READONLY);
    }

  /* Check for matching section type flags for sections we care about.
     A section without contents can have SEC_LOAD == 0, but we still
     want it attached to a sane section so the symbols appear as
     expected.  */

  if ((flags & (SEC_ALLOC | SEC_READONLY)) != SEC_READONLY)
    for (i = 0; i < sizeof (holds) / sizeof (holds[0]); i++)
      if ((flags & holds[i].nonzero_flags) != 0)
	{
	  place = &holds[i].orphansave;
	  if (place->os == NULL)
	    place->os = lang_output_section_find (place->name);
	  break;
	}

  if (place == NULL)
    {
      /* For other combinations, we have to give up, except we make
	 sure not to place the orphan section after the
	 linker-generated register section; that'd make it continue
	 the reg section and we never want that to happen for orphan
	 sections.  */
      lang_output_section_statement_type *before;
      lang_output_section_statement_type *lookup;
      static struct orphan_save hold_nonreg =
	{
	  NULL,
	  SEC_READONLY,
	  0, 0, 0, 0
	};

      if (hold_nonreg.os == NULL)
	{
	  before = lang_output_section_find (MMIX_REG_CONTENTS_SECTION_NAME);

	  /* If we have no such section, all fine; we don't care where
	     it's placed.  */
	  if (before == NULL)
	    return NULL;

	  /* We have to find the oss before this one, so we can use that as
	     "after".  */
	  for (lookup = (void *) lang_os_list.head;
	       lookup != NULL && lookup->next != before;
	       lookup = lookup->next)
	    ;

	  hold_nonreg.os = lookup;
	}

      place = &hold_nonreg;
    }

  after = place->os;
  if (after == NULL)
    return NULL;

  /* If there's an output section by *this* name, we'll use it, regardless
     of actual section flags, in contrast to what's done in elf.em.  */
  os = lang_insert_orphan (s, secname, 0, after, place, NULL, NULL);

  return os;
}

/* Remove the spurious settings of SEC_RELOC that make it to the output at
   link time.  We are as confused as elflink.h:elf_bfd_final_link, and
   paper over the bug similarly.  */

static void
mmo_wipe_sec_reloc_flag (bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
			 void *ptr ATTRIBUTE_UNUSED)
{
  bfd_set_section_flags (sec, bfd_section_flags (sec) & ~SEC_RELOC);
}

/* Iterate with bfd_map_over_sections over mmo_wipe_sec_reloc_flag... */

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  bfd_map_over_sections (link_info.output_bfd, mmo_wipe_sec_reloc_flag, NULL);
  ldelf_map_segments (false);
}

/* To get on-demand global register allocation right, we need to parse the
   relocs, like what happens when linking to ELF.  It needs to be done
   before all input sections are supposed to be present.  When linking to
   ELF, it's done when reading symbols.  When linking to mmo, we do it
   when all input files are seen, which is equivalent.  */

static void
mmo_after_open (void)
{
  /* When there's a mismatch between the output format and the emulation
     (using weird combinations like "-m mmo --oformat elf64-mmix" for
     example), we'd count relocs twice because they'd also be counted
     along the usual route for ELF-only linking, which would lead to an
     internal accounting error.  */
  if (bfd_get_flavour (link_info.output_bfd) != bfd_target_elf_flavour)
    {
      LANG_FOR_EACH_INPUT_STATEMENT (is)
	{
	  if (bfd_get_flavour (is->the_bfd) == bfd_target_elf_flavour
	      && !_bfd_mmix_check_all_relocs (is->the_bfd, &link_info))
	    einfo (_("%X%P: internal problems scanning %pB after opening it"),
		   is->the_bfd);
	}
    }
  after_open_default ();
}
EOF

LDEMUL_PLACE_ORPHAN=mmo_place_orphan
LDEMUL_AFTER_OPEN=mmo_after_open
