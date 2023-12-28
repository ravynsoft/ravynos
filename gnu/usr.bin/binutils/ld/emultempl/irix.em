# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2004-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra irix specific
# functions.

fragment <<EOF

/* The native IRIX linker will always create a DT_SONAME for shared objects.
   While this shouldn't really be necessary for ABI conformance, some versions
   of the native linker will segfault if the tag is missing.  */

static void
irix_after_open (void)
{
  if (bfd_link_dll (&link_info) && command_line.soname == 0)
    command_line.soname
      = (char *) lbasename (bfd_get_filename (link_info.output_bfd));

  gld${EMULATION_NAME}_after_open ();
}
EOF

LDEMUL_AFTER_OPEN=irix_after_open
source_em "${srcdir}/emultempl/mipself.em"
