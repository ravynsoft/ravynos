# This shell script emits a C file. -*- C -*-
# Copyright (C) 2019-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines x86 specific routines.
#
fragment <<EOF

#include "ldlex.h"
#include "elf-linker-x86.h"

static struct elf_linker_x86_params params;

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */

static void
elf_x86_create_output_section_statements (void)
{
  _bfd_elf_linker_x86_set_options (&link_info, &params);
}

EOF

LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=elf_x86_create_output_section_statements

if test -n "$CALL_NOP_BYTE"; then

fragment <<EOF

static void
elf_x86_before_parse (void)
{
  params.call_nop_byte = $CALL_NOP_BYTE;

  gld${EMULATION_NAME}_before_parse ();
}

EOF

LDEMUL_BEFORE_PARSE=elf_x86_before_parse
fi
