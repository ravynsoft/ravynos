# This shell script emits a C file. -*- C -*-
# Copyright (C) 2017-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the license, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3. If not,
# see <http://www.gnu.org/licenses/>.
#

# This file is sourced from elf-generic.em, and defines S/390
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf-s390.h"

static struct s390_elf_params params = { 0 };

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
s390_elf_create_output_section_statements (void)
{
  if (!bfd_elf_s390_set_options (&link_info, &params))
    einfo (_("%F%P: can not init BFD: %E\n"));
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_PGSTE	301
'

PARSE_AND_LIST_LONGOPTS='
  { "s390-pgste", no_argument, NULL, OPTION_PGSTE},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --s390-pgste                Tell the kernel to "
		   "allocate 4k page tables\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_PGSTE:
      params.pgste = 1;
      break;
'

LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=s390_elf_create_output_section_statements
