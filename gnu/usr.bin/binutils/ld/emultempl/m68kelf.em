# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2000-2023 Free Software Foundation, Inc.
#   Written by Michael Sokolov <msokolov@ivan.Harhan.ORG>, based on armelf.em
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


# This file is sourced from elf.em, and defines some extra routines for m68k
# embedded systems using ELF and for some other systems using m68k ELF.  While
# it is sourced from elf.em for all m68k ELF configurations, here we include
# only the features we want depending on the configuration.

case ${target} in
  m68*-*-elf)
    echo "#define SUPPORT_EMBEDDED_RELOCS" >>e${EMULATION_NAME}.c
    ;;
esac

case ${target} in
  *-linux*)
# Don't use multi-GOT by default due to glibc linker's assumption
# that GOT pointer points to GOT[0].
#   got_handling_target_default=GOT_HANDLING_MULTIGOT
    got_handling_target_default=GOT_HANDLING_SINGLE
    ;;
  *)
    got_handling_target_default=GOT_HANDLING_SINGLE
    ;;
esac

fragment <<EOF

#include "elf32-m68k.h"

#define GOT_HANDLING_SINGLE   (0)
#define GOT_HANDLING_NEGATIVE (1)
#define GOT_HANDLING_MULTIGOT (2)
#define GOT_HANDLING_TARGET_DEFAULT ${got_handling_target_default}

/* How to generate GOT.  */
static int got_handling = GOT_HANDLING_DEFAULT;

#ifdef SUPPORT_EMBEDDED_RELOCS
static void check_sections (bfd *, asection *, void *);
#endif

/* This function is run after all the input files have been opened.  */

static void
m68k_elf_after_open (void)
{
  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_open ();

#ifdef SUPPORT_EMBEDDED_RELOCS
  if (command_line.embedded_relocs
      && (!bfd_link_relocatable (&link_info)))
    {
      bfd *abfd;

      /* In the embedded relocs mode we create a .emreloc section for each
	 input file with a nonzero .data section.  The BFD backend will fill in
	 these sections with magic numbers which can be used to relocate the
	 data section at run time.  */
      for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
	{
	  asection *datasec;

	  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
	    einfo (_("%F%P: %pB: all input objects must be ELF "
		     "for --embedded-relocs\n"));

	  datasec = bfd_get_section_by_name (abfd, ".data");

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

	      relsec = bfd_make_section_with_flags (abfd, ".emreloc",
						    (SEC_ALLOC
						    | SEC_LOAD
						    | SEC_HAS_CONTENTS
						    | SEC_IN_MEMORY));
	      if (relsec == NULL
		  || !bfd_set_section_alignment (relsec, 2)
		  || !bfd_set_section_size (relsec, datasec->reloc_count * 12))
		einfo (_("%F%P: %pB: can not create .emreloc section: %E\n"));
	    }

	  /* Double check that all other data sections are empty, as is
	     required for embedded PIC code.  */
	  bfd_map_over_sections (abfd, check_sections, datasec);
	}
    }
#endif /* SUPPORT_EMBEDDED_RELOCS */
}

#ifdef SUPPORT_EMBEDDED_RELOCS
/* Check that of the data sections, only the .data section has
   relocs.  This is called via bfd_map_over_sections.  */

static void
check_sections (bfd *abfd, asection *sec, void *datasec)
{
  if ((bfd_section_flags (sec) & SEC_DATA)
      && sec != datasec
      && sec->reloc_count != 0)
    einfo (_("%X%P: %pB: section %s has relocs; can not use --embedded-relocs\n"),
	   abfd, bfd_section_name (sec));
}

#endif /* SUPPORT_EMBEDDED_RELOCS */

/* This function is called after the section sizes and offsets have
   been set.  */

static void
m68k_elf_after_allocation (void)
{
  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_allocation ();

#ifdef SUPPORT_EMBEDDED_RELOCS
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

	  datasec = bfd_get_section_by_name (abfd, ".data");

	  if (datasec == NULL || datasec->reloc_count == 0)
	    continue;

	  relsec = bfd_get_section_by_name (abfd, ".emreloc");
	  ASSERT (relsec != NULL);

	  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
	    {
	      if (! bfd_m68k_elf32_create_embedded_relocs (abfd, &link_info,
							   datasec, relsec,
							   &errmsg))
		{
		  if (errmsg == NULL)
		    einfo (_("%X%P: %pB: can not create "
			     "runtime reloc information: %E\n"),
			   abfd);
		  else
		    einfo (_("%X%P: %pB: can not create "
			     "runtime reloc information: %s\n"),
			   abfd, errmsg);
		}
	    }
	  else
	    abort ();
	}
    }
#endif /* SUPPORT_EMBEDDED_RELOCS */
}

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */

static void
elf_m68k_create_output_section_statements (void)
{
  bfd_elf_m68k_set_target_options (&link_info, got_handling);
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_GOT	301
'

PARSE_AND_LIST_LONGOPTS='
  { "got", required_argument, NULL, OPTION_GOT},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --got=<type>                Specify GOT handling scheme\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_GOT:
      if (strcmp (optarg, "target") == 0)
	got_handling = GOT_HANDLING_TARGET_DEFAULT;
      else if (strcmp (optarg, "single") == 0)
	got_handling = 0;
      else if (strcmp (optarg, "negative") == 0)
	got_handling = 1;
      else if (strcmp (optarg, "multigot") == 0)
	got_handling = 2;
      else
	einfo (_("%P: unrecognized --got argument '\''%s'\''\n"), optarg);
      break;
'

# We have our own after_open and after_allocation functions, but they call
# the standard routines, so give them a different name.
LDEMUL_AFTER_OPEN=m68k_elf_after_open
LDEMUL_AFTER_ALLOCATION=m68k_elf_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=elf_m68k_create_output_section_statements
