# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra bfin-elf
# specific routines.
#
fragment <<EOF

#include "elf-bfd.h"
#include "elf32-bfin.h"

/* Whether to put code in Blackfin L1 SRAM.  */
extern bool elf32_bfin_code_in_l1;

/* Whether to put (writable) data in Blackfin L1 SRAM.  */
extern bool elf32_bfin_data_in_l1;

EOF


# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_CODE_IN_L1		300
#define OPTION_DATA_IN_L1		301
'

PARSE_AND_LIST_LONGOPTS='
  { "code-in-l1", no_argument, NULL, OPTION_CODE_IN_L1 },
  { "data-in-l1", no_argument, NULL, OPTION_DATA_IN_L1 },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("\
  --code-in-l1                Put code in L1\n"));
  fprintf (file, _("\
  --data-in-l1                Put data in L1\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_CODE_IN_L1:
      elf32_bfin_code_in_l1 = true;
      break;
    case OPTION_DATA_IN_L1:
      elf32_bfin_data_in_l1 = true;
      break;
'
