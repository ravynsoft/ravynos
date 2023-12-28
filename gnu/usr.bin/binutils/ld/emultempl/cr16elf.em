# This shell script emits a C file. -*- C -*-
# Copyright (C) 2007-2023 Free Software Foundation, Inc.
# Contributed by M R Swami Reddy <MR.Swami.Reddy@nsc.com>
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

# This file is sourced from elf.em, and defines extra cr16-elf
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf32-cr16.h"

static void check_sections (bfd *, asection *, void *);


/* This function is run after all the input files have been opened.  */

static void
cr16_elf_after_open (void)
{
  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_open ();

  if (command_line.embedded_relocs
      && !bfd_link_relocatable (&link_info))
    {
      bfd *abfd;

      /* In the embedded relocs mode we create a .emreloc section for each
	 input file with a nonzero .data section.  The BFD backend will fill in
	 these sections with magic numbers which can be used to relocate the
	 data section at run time.  */
      for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
	{
	  asection *datasec;

	  /* As first-order business, make sure that each input BFD is either
	     COFF or ELF.  We need to call a special BFD backend function to
	     generate the embedded relocs, and we have such functions only for
	     COFF and ELF.  */
	  if (bfd_get_flavour (abfd) != bfd_target_coff_flavour
	      && bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	    einfo (_("%F%P: %pB: all input objects must be COFF or ELF "
		     "for --embedded-relocs\n"));

	  datasec = bfd_get_section_by_name (abfd, ".data.rel");

	  /* Note that we assume that the reloc_count field has already
	     been set up.  We could call bfd_get_reloc_upper_bound, but
	     that returns the size of a memory buffer rather than a reloc
	     count.  We do not want to call bfd_canonicalize_reloc,
	     because although it would always work it would force us to
	     read in the relocs into BFD canonical form, which would waste
	     a significant amount of time and memory.  */
	  if (datasec != NULL && datasec->reloc_count > 0)
	    {
	      asection *relsec;

	      relsec = bfd_make_section (abfd, ".emreloc");
	      if (relsec == NULL
		  || !bfd_set_section_flags (relsec, (SEC_ALLOC
						      | SEC_LOAD
						      | SEC_HAS_CONTENTS
						      | SEC_IN_MEMORY))
		  || !bfd_set_section_alignment (relsec, 2)
		  || !bfd_set_section_size (relsec, datasec->reloc_count * 8))
		einfo (_("%F%P: %pB: can not create .emreloc section: %E\n"));
	    }

	  /* Double check that all other data sections are empty, as is
	     required for embedded PIC code.  */
	  bfd_map_over_sections (abfd, check_sections, datasec);
	}
    }
}

/* Check that of the data sections, only the .data section has
   relocs.  This is called via bfd_map_over_sections.  */

static void
check_sections (bfd *abfd, asection *sec, void *datasec)
{
  if ((strncmp (bfd_section_name (sec), ".data.rel", 9) == 0)
      && sec != datasec
      && sec->reloc_count == 0 )
    einfo (_("%X%P: %pB: section %s has relocs; can not use --embedded-relocs\n"),
	   abfd, bfd_section_name (sec));
}

static void
cr16elf_after_parse (void)
{
  /* Always behave as if called with --sort-common command line
     option.
     This is to emulate the CRTools' method of keeping variables
     of different alignment in separate sections.  */
  config.sort_common = true;

  /* Don't create a demand-paged executable, since this feature isn't
     meaninful in CR16 embedded systems. Moreover, when magic_demand_paged
     is true the link sometimes fails.  */
  config.magic_demand_paged = false;

  ldelf_after_parse ();
}

/* This is called after the sections have been attached to output
   sections, but before any sizes or addresses have been set.  */

static void
cr16elf_before_allocation (void)
{
  /* Call the default first.  */
  gld${EMULATION_NAME}_before_allocation ();

  if (command_line.embedded_relocs
      && (!bfd_link_relocatable (&link_info)))
    {

      bfd *abfd;

      /* If we are generating embedded relocs, call a special BFD backend
	 routine to do the work.  */
      for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
	{
	  asection *datasec, *relsec;
	  char *errmsg;

	  datasec = bfd_get_section_by_name (abfd, ".data.rel");

	  if (datasec == NULL || datasec->reloc_count == 0)
	    continue;

	  relsec = bfd_get_section_by_name (abfd, ".emreloc");
	  ASSERT (relsec != NULL);

	  if (! bfd_cr16_elf32_create_embedded_relocs (abfd, &link_info,
						       datasec, relsec,
						       &errmsg))
	    {
	      if (errmsg == NULL)
		einfo (_("%X%P: %pB: can not create runtime reloc information: %E\n"),
		       abfd);
	      else
		einfo (_("%X%P: %pB: can not create runtime reloc information: %s\n"),
		       abfd, errmsg);
	    }
	}
    }

  /* Enable relaxation by default if the "--no-relax" option was not
     specified.  This is done here instead of in the before_parse hook
     because there is a check in main() to prohibit use of --relax and
     -r together.  */
  if (RELAXATION_DISABLED_BY_DEFAULT)
    ENABLE_RELAXATION;
}

EOF

# Put these extra cr16-elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_AFTER_OPEN=cr16_elf_after_open
LDEMUL_AFTER_PARSE=cr16elf_after_parse
LDEMUL_BEFORE_ALLOCATION=cr16elf_before_allocation
