# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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


# This file is sourced from elf.em, and defines extra avr-elf specific
# routines.  It is used to generate the trampolines for the avr6 family
# of devices where one needs to address the issue that it is not possible
# to reach the whole program memory by using 16 bit pointers.

fragment <<EOF

#include "elf32-avr.h"
#include "ldctor.h"
#include "elf/avr.h"

/* The fake file and it's corresponding section meant to hold
   the linker stubs if needed.  */

static lang_input_statement_type *stub_file;
static asection *avr_stub_section;

/* Variables set by the command-line parameters and transferred
   to the bfd without use of global shared variables.  */

static bool avr_no_stubs = false;
static bool avr_debug_relax = false;
static bool avr_debug_stubs = false;
static bool avr_replace_call_ret_sequences = true;
static bfd_vma avr_pc_wrap_around = 0x10000000;

/* Transfers information to the bfd frontend.  */

static void
avr_elf_set_global_bfd_parameters (void)
{
  elf32_avr_setup_params (& link_info,
			  stub_file->the_bfd,
			  avr_stub_section,
			  avr_no_stubs,
			  avr_debug_stubs,
			  avr_debug_relax,
			  avr_pc_wrap_around,
			  avr_replace_call_ret_sequences);
}


/* Makes a conservative estimate of the trampoline section size that could
   be corrected later on.  */

static void
avr_elf_${EMULATION_NAME}_before_allocation (void)
{
  int ret;

  gld${EMULATION_NAME}_before_allocation ();

  if (bfd_get_flavour (link_info.output_bfd) != bfd_target_elf_flavour)
    {
      avr_no_stubs = true;
      return;
    }

  /* We only need stubs for avr6, avrxmega6, and avrxmega7.  */
  if (strcmp ("${EMULATION_NAME}", "avr6") != 0
      && strcmp ("${EMULATION_NAME}", "avrxmega6") != 0
      && strcmp ("${EMULATION_NAME}", "avrxmega7") != 0)
    avr_no_stubs = true;

  avr_elf_set_global_bfd_parameters ();

  /* If generating a relocatable output file, then
     we don't  have to generate the trampolines.  */
  if (bfd_link_relocatable (&link_info))
    avr_no_stubs = true;

  if (avr_no_stubs)
    return;

  ret = elf32_avr_setup_section_lists (link_info.output_bfd, &link_info);

  if (ret < 0)
    einfo (_("%X%P: can not setup the input section list: %E\n"));

  if (ret <= 0)
    return;

  /* Call into the BFD backend to do the real "stub"-work.  */
  if (! elf32_avr_size_stubs (link_info.output_bfd, &link_info, true))
    einfo (_("%X%P: can not size stub section: %E\n"));
}

/* This is called before the input files are opened.  We create a new
   fake input file to hold the stub section and generate the section itself.  */

static void
avr_elf_create_output_section_statements (void)
{
  flagword flags;

  if (bfd_get_flavour (link_info.output_bfd) != bfd_target_elf_flavour)
    {
      einfo (_("%F%P: error: cannot change output format "
	       "whilst linking %s binaries\n"), "AVR");
      return;
    }

  stub_file = lang_add_input_file ("linker stubs",
				   lang_input_file_is_fake_enum,
				   NULL);

  stub_file->the_bfd = bfd_create ("linker stubs", link_info.output_bfd);
  if (stub_file->the_bfd == NULL
      || !bfd_set_arch_mach (stub_file->the_bfd,
			     bfd_get_arch (link_info.output_bfd),
			     bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%X%P: can not create stub BFD: %E\n"));
      return;
    }
  stub_file->the_bfd->flags |= BFD_LINKER_CREATED;

  /* Now we add the stub section.  */

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
	   | SEC_HAS_CONTENTS | SEC_RELOC | SEC_IN_MEMORY | SEC_KEEP);
  avr_stub_section = bfd_make_section_anyway_with_flags (stub_file->the_bfd,
							 ".trampolines",
							 flags);
  if (avr_stub_section == NULL)
    goto err_ret;

  avr_stub_section->alignment_power = 1;

  ldlang_add_file (stub_file);

  return;

 err_ret:
  einfo (_("%X%P: can not make stub section: %E\n"));
  return;
}

/* Re-calculates the size of the stubs so that we won't waste space.  */

static void
avr_elf_after_allocation (void)
{
  if (!avr_no_stubs && ! RELAXATION_ENABLED)
    {
      /* If relaxing, elf32_avr_size_stubs will be called from
	 elf32_avr_relax_section.  */
      if (!elf32_avr_size_stubs (link_info.output_bfd, &link_info, true))
	einfo (_("%X%P: can not size stub section: %E\n"));
    }

  gld${EMULATION_NAME}_after_allocation ();

  /* Now build the linker stubs.  */
  if (!avr_no_stubs)
    {
      if (!elf32_avr_build_stubs (&link_info))
	einfo (_("%X%P: can not build stubs: %E\n"));
    }
}

static void
avr_elf_before_parse (void)
{
  /* Don't create a demand-paged executable, since this feature isn't
     meaningful in AVR. */
  config.magic_demand_paged = false;

  gld${EMULATION_NAME}_before_parse ();
}

static void
avr_finish (void)
{
  bfd *abfd;
  bool avr_link_relax;

  if (bfd_link_relocatable (&link_info))
    {
      avr_link_relax = true;
      for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
	{
	  /* Don't let the linker stubs prevent the final object being
	     marked as link-relax ready.  */
	  if ((elf_elfheader (abfd)->e_flags
	       & EF_AVR_LINKRELAX_PREPARED) == 0
	      && abfd != stub_file->the_bfd)
	    {
	      avr_link_relax = false;
	      break;
	    }
	}
    }
  else
    {
      avr_link_relax = RELAXATION_ENABLED;
    }

  abfd = link_info.output_bfd;

  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour)
    {
      if (avr_link_relax)
	elf_elfheader (abfd)->e_flags |= EF_AVR_LINKRELAX_PREPARED;
      else
	elf_elfheader (abfd)->e_flags &= ~EF_AVR_LINKRELAX_PREPARED;
    }

  finish_default ();
}
EOF


PARSE_AND_LIST_PROLOGUE='

#define OPTION_NO_CALL_RET_REPLACEMENT 301
#define OPTION_PMEM_WRAP_AROUND	       302
#define OPTION_NO_STUBS		       303
#define OPTION_DEBUG_STUBS	       304
#define OPTION_DEBUG_RELAX	       305
'

PARSE_AND_LIST_LONGOPTS='
  { "no-call-ret-replacement", no_argument,
    NULL, OPTION_NO_CALL_RET_REPLACEMENT},
  { "pmem-wrap-around", required_argument,
    NULL, OPTION_PMEM_WRAP_AROUND},
  { "no-stubs", no_argument,
    NULL, OPTION_NO_STUBS},
  { "debug-stubs", no_argument,
    NULL, OPTION_DEBUG_STUBS},
  { "debug-relax", no_argument,
    NULL, OPTION_DEBUG_RELAX},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --pmem-wrap-around=<val>    "
		   "Make the linker relaxation machine assume that a\n"
		   "                              "
		   "  program counter wrap-around occurs at address\n"
		   "                              "
		   "  <val>.  Supported values: 8k, 16k, 32k and 64k.\n"));
  fprintf (file, _("  --no-call-ret-replacement   "
		   "The relaxation machine normally will\n"
		   "                              "
		   "  substitute two immediately following call/ret\n"
		   "                              "
		   "  instructions by a single jump instruction.\n"
		   "                              "
		   "  This option disables this optimization.\n"));
  fprintf (file, _("  --no-stubs                  "
		   "If the linker detects to attempt to access\n"
		   "                              "
		   "  an instruction beyond 128k by a reloc that\n"
		   "                              "
		   "  is limited to 128k max, it inserts a jump\n"
		   "                              "
		   "  stub. You can de-active this with this switch.\n"));
  fprintf (file, _("  --debug-stubs               "
		   "Used for debugging avr-ld.\n"));
  fprintf (file, _("  --debug-relax               "
		   "Used for debugging avr-ld.\n"));
'

PARSE_AND_LIST_ARGS_CASES='

    case OPTION_PMEM_WRAP_AROUND:
      {
	/* This variable is defined in the bfd library.  */
	if ((!strcmp (optarg,"32k"))      || (!strcmp (optarg,"32K")))
	  avr_pc_wrap_around = 32768;
	else if ((!strcmp (optarg,"8k"))  || (!strcmp (optarg,"8K")))
	  avr_pc_wrap_around = 8192;
	else if ((!strcmp (optarg,"16k")) || (!strcmp (optarg,"16K")))
	  avr_pc_wrap_around = 16384;
	else if ((!strcmp (optarg,"64k")) || (!strcmp (optarg,"64K")))
	  avr_pc_wrap_around = 0x10000;
	else
	  return false;
      }
      break;

    case OPTION_DEBUG_STUBS:
      avr_debug_stubs = true;
      break;

    case OPTION_DEBUG_RELAX:
      avr_debug_relax = true;
      break;

    case OPTION_NO_STUBS:
      avr_no_stubs = true;
      break;

    case OPTION_NO_CALL_RET_REPLACEMENT:
      {
	/* This variable is defined in the bfd library.  */
	avr_replace_call_ret_sequences = false;
      }
      break;
'

#
# Put these extra avr-elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_BEFORE_PARSE=avr_elf_before_parse
LDEMUL_BEFORE_ALLOCATION=avr_elf_${EMULATION_NAME}_before_allocation
LDEMUL_AFTER_ALLOCATION=avr_elf_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=avr_elf_create_output_section_statements
LDEMUL_FINISH=avr_finish
