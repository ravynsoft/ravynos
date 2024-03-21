# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2020-2023 Free Software Foundation, Inc.
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

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
rx_linux_create_output_section_statements (void)
{
  extern void bfd_elf32_rx_set_target_flags (bool, bool);

  bfd_elf32_rx_set_target_flags (false, false);
}

EOF
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=rx_linux_create_output_section_statements

