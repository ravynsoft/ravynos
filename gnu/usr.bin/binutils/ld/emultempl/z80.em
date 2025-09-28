# This shell script emits C code -*- C -*-
# to keep track of the machine type of Z80 object files
# It does some substitutions.
#   Copyright (C) 2005-2023 Free Software Foundation, Inc.
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

if [ x"${EMULATION_NAME}" = x"elf32z80" ]; then
  fragment <<EOF
#include "elf/z80.h"
EOF
else
  fragment <<EOF
static void
gld${EMULATION_NAME}_after_open (void)
{
}
EOF
fi

fragment <<EOF
/* --- \begin{z80.em} */

/* Set the machine type of the output file based on types of inputs.  */
static void
z80_after_open (void)
{
  bfd *abfd;

  /* For now, make sure all object files are of the same architecture.
     We may try to merge object files with different architecture together.  */
  for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
    {
      const bfd_arch_info_type *info;
      info = bfd_arch_get_compatible (link_info.output_bfd, abfd, false);
      if (info == NULL)
	einfo (_("%F%P: %pB: Instruction sets of object files incompatible\n"),
	       abfd);
      else
        bfd_set_arch_info (link_info.output_bfd, info);
    }

  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_open ();
}

/* --- \end{z80.em} */
EOF

LDEMUL_AFTER_OPEN=z80_after_open
