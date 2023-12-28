# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2003-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra alpha
# specific routines.
#
fragment <<EOF

#include "elf/internal.h"
#include "elf/alpha.h"
#include "elf-bfd.h"

static bool limit_32bit;

extern bool elf64_alpha_use_secureplt;


/* Set the start address as in the Tru64 ld.  */
#define ALPHA_TEXT_START_32BIT 0x12000000

static void
alpha_after_open (void)
{
  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && elf_object_id (link_info.output_bfd) == ALPHA_ELF_DATA)
    {
      unsigned int num_plt;
      lang_output_section_statement_type *os;
      lang_output_section_statement_type *plt_os[2];

      num_plt = 0;
      for (os = (void *) lang_os_list.head;
	   os != NULL;
	   os = os->next)
	{
	  if (os->constraint == SPECIAL && strcmp (os->name, ".plt") == 0)
	    {
	      if (num_plt < 2)
		plt_os[num_plt] = os;
	      ++num_plt;
	    }
	}

      if (num_plt == 2)
	{
	  plt_os[0]->constraint = elf64_alpha_use_secureplt ? 0 : -1;
	  plt_os[1]->constraint = elf64_alpha_use_secureplt ? -1 : 0;
	}
    }

  gld${EMULATION_NAME}_after_open ();
}

static void
alpha_after_parse (void)
{
  link_info.relax_pass = 2;
  if (limit_32bit
      && !bfd_link_pic (&link_info)
      && !bfd_link_relocatable (&link_info))
    lang_section_start (".interp",
			exp_binop ('+',
				   exp_intop (ALPHA_TEXT_START_32BIT),
				   exp_nameop (SIZEOF_HEADERS, NULL)),
			NULL);

  ldelf_after_parse ();
}

static void
alpha_before_allocation (void)
{
  /* Call main function; we're just extending it.  */
  gld${EMULATION_NAME}_before_allocation ();

  /* Add -relax if -O, not -r, and not explicitly disabled.  */
  if (link_info.optimize
      && !bfd_link_relocatable (&link_info)
      && ! RELAXATION_DISABLED_BY_USER)
    ENABLE_RELAXATION;
}

static void
alpha_finish (void)
{
  if (limit_32bit)
    elf_elfheader (link_info.output_bfd)->e_flags |= EF_ALPHA_32BIT;

  finish_default ();
}
EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_TASO		300
#define OPTION_SECUREPLT	(OPTION_TASO + 1)
#define OPTION_NO_SECUREPLT	(OPTION_SECUREPLT + 1)
'

PARSE_AND_LIST_LONGOPTS='
  { "taso", no_argument, NULL, OPTION_TASO },
  { "secureplt", no_argument, NULL, OPTION_SECUREPLT },
  { "no-secureplt", no_argument, NULL, OPTION_NO_SECUREPLT },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("\
  --taso                      Load executable in the lower 31-bit addressable\n\
                                virtual address range\n"));
  fprintf (file, _("\
  --secureplt                 Force PLT in text segment\n"));
  fprintf (file, _("\
  --no-secureplt              Force PLT in data segment\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_TASO:
      limit_32bit = 1;
      break;
    case OPTION_SECUREPLT:
      elf64_alpha_use_secureplt = true;
      break;
    case OPTION_NO_SECUREPLT:
      elf64_alpha_use_secureplt = false;
      break;
'

# Put these extra alpha routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_AFTER_OPEN=alpha_after_open
LDEMUL_AFTER_PARSE=alpha_after_parse
LDEMUL_BEFORE_ALLOCATION=alpha_before_allocation
LDEMUL_FINISH=alpha_finish
