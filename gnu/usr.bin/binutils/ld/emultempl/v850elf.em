# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2013-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra cpu specific
# features.
#
fragment <<EOF
#include "ldctor.h"
#include "elf32-v850.h"

static bool
is_v850_target (void)
{
  extern const bfd_target v850_elf32_vec;
  extern const bfd_target v800_elf32_vec;

  return link_info.output_bfd->xvec == & v850_elf32_vec
      || link_info.output_bfd->xvec == & v800_elf32_vec;
}

/* Create our note section.  */

static void
v850_after_open (void)
{
  if (is_v850_target ()
      && !bfd_link_relocatable (&link_info)
      && link_info.input_bfds != NULL
      && ! v850_elf_create_sections (& link_info))
	einfo (_("%X%P: can not create note section: %E\n"));

  gld${EMULATION_NAME}_after_open ();
}

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */

static void
v850_create_output_section_statements (void)
{
  /* See PR 22419 for an example of why this is necessary.  */
  if (strstr (bfd_get_target (link_info.output_bfd), "v850") == NULL)
    {
      /* The V850 backend needs special fields in the output hash structure.
	 These will only be created if the output format is an arm format,
	 hence we do not support linking and changing output formats at the
	 same time.  Use a link followed by objcopy to change output formats.  */
      einfo (_("%F%P: error: cannot change output format"
	       " whilst linking %s binaries\n"), "V850");
      return;
    }
}


EOF

LDEMUL_AFTER_OPEN=v850_after_open
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=v850_create_output_section_statements
