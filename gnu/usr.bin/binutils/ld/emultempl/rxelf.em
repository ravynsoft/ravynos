# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2009-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra rx-elf
# specific routines.
#
test -z "$TARGET2_TYPE" && TARGET2_TYPE="rel"
fragment <<EOF

#include "elf32-rx.h"

static bool no_flag_mismatch_warnings = true;
static bool ignore_lma = true;

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
rx_elf_create_output_section_statements (void)
{
  extern void bfd_elf32_rx_set_target_flags (bool, bool);

  bfd_elf32_rx_set_target_flags (no_flag_mismatch_warnings, ignore_lma);
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_NO_FLAG_MISMATCH_WARNINGS	301
#define OPTION_IGNORE_LMA			302
#define OPTION_NO_IGNORE_LMA			303
#define OPTION_FLAG_MISMATCH_WARNINGS		304
'

PARSE_AND_LIST_LONGOPTS='
  { "no-flag-mismatch-warnings", no_argument, NULL, OPTION_NO_FLAG_MISMATCH_WARNINGS},
  { "flag-mismatch-warnings", no_argument, NULL, OPTION_FLAG_MISMATCH_WARNINGS},
  { "ignore-lma", no_argument, NULL, OPTION_IGNORE_LMA},
  { "no-ignore-lma", no_argument, NULL, OPTION_NO_IGNORE_LMA},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --no-flag-mismatch-warnings Don'\''t warn about objects with incompatible\n"
		   "                                endian or dsp settings\n"));
  fprintf (file, _("  --flag-mismatch-warnings    Warn about objects with incompatible\n"
		   "                                endian, dsp or ABI settings\n"));
  fprintf (file, _("  --ignore-lma                Ignore segment LMAs [default]\n"
                   "                                (for Renesas Tools compatibility)\n"));
  fprintf (file, _("  --no-ignore-lma             Don'\''t ignore segment LMAs\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_NO_FLAG_MISMATCH_WARNINGS:
      no_flag_mismatch_warnings = true;
      break;

    case OPTION_FLAG_MISMATCH_WARNINGS:
      no_flag_mismatch_warnings = false;
      break;

    case OPTION_IGNORE_LMA:
      ignore_lma = true;
      break;

    case OPTION_NO_IGNORE_LMA:
      ignore_lma = false;
      break;
'

LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=rx_elf_create_output_section_statements

LDEMUL_EXTRA_MAP_FILE_TEXT=rx_additional_link_map_text
