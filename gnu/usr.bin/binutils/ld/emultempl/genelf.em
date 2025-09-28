# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

# This file is sourced from generic.em
#
fragment <<EOF
#include "elf-bfd.h"
#include "ldelfgen.h"

EOF
source_em ${srcdir}/emultempl/elf-generic.em
fragment <<EOF

static void
gld${EMULATION_NAME}_after_open (void)
{
  bfd *ibfd;
  asection *sec;
  asymbol **syms;

  after_open_default ();

  if (bfd_link_relocatable (&link_info))
    for (ibfd = link_info.input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
      if ((syms = bfd_get_outsymbols (ibfd)) != NULL
	  && bfd_get_flavour (ibfd) == bfd_target_elf_flavour)
	for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	  if ((sec->flags & (SEC_GROUP | SEC_LINKER_CREATED)) == SEC_GROUP)
	    {
	      struct bfd_elf_section_data *sec_data = elf_section_data (sec);
	      struct bfd_symbol *sym = syms[sec_data->this_hdr.sh_info - 1];
	      elf_group_id (sec) = sym;
	      sym->flags |= BSF_KEEP;
	    }
}

static void
gld${EMULATION_NAME}_before_allocation (void)
{
  if (bfd_link_relocatable (&link_info)
      && !_bfd_elf_size_group_sections (&link_info))
    einfo (_("%X%P: can not size group sections: %E\n"));
  before_allocation_default ();
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  ldelf_map_segments (false);
}
EOF
# Put these extra routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_AFTER_OPEN=gld${EMULATION_NAME}_after_open
LDEMUL_BEFORE_ALLOCATION=gld${EMULATION_NAME}_before_allocation
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
