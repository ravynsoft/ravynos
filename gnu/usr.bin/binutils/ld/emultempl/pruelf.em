# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2016-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra pru-elf
# specific routines.
#
fragment <<EOF

#include "ldctor.h"

/* This is called after the sections have been attached to output
   sections, but before any sizes or addresses have been set.  */

static void
pruelf_before_allocation (void)
{
  /* Call the default first.  */
  gld${EMULATION_NAME}_before_allocation ();

  /* Enable relaxation by default if the "--no-relax" option was not
     specified.  This is done here instead of in the before_parse hook
     because there is a check in main() to prohibit use of --relax and
     -r together.  */
  if (RELAXATION_DISABLED_BY_DEFAULT)
    ENABLE_RELAXATION;
}

EOF

# Put these extra pru-elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_BEFORE_ALLOCATION=pruelf_before_allocation
