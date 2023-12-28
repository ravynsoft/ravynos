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

fragment <<EOF

#include "ldmain.h"
#include "ldctor.h"
#include "elf/riscv.h"
#include "elfxx-riscv.h"

static struct riscv_elf_params params = { .relax_gp = 1 };
EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.  */
PARSE_AND_LIST_PROLOGUE=${PARSE_AND_LIST_PROLOGUE}'
enum risccv_opt
{
  OPTION_RELAX_GP = 321,
  OPTION_NO_RELAX_GP,
};
'

PARSE_AND_LIST_LONGOPTS=${PARSE_AND_LIST_LONGOPTS}'
    { "relax-gp", no_argument, NULL, OPTION_RELAX_GP },
    { "no-relax-gp", no_argument, NULL, OPTION_NO_RELAX_GP },
'

PARSE_AND_LIST_OPTIONS=${PARSE_AND_LIST_OPTIONS}'
  fprintf (file, _("  --relax-gp                  Perform GP relaxation\n"));
  fprintf (file, _("  --no-relax-gp               Don'\''t perform GP relaxation\n"));
'

PARSE_AND_LIST_ARGS_CASES=${PARSE_AND_LIST_ARGS_CASES}'
    case OPTION_RELAX_GP:
      params.relax_gp = 1;
      break;

    case OPTION_NO_RELAX_GP:
      params.relax_gp = 0;
      break;
'

fragment <<EOF
static void
riscv_elf_before_allocation (void)
{
  gld${EMULATION_NAME}_before_allocation ();

  if (link_info.discard == discard_sec_merge)
    link_info.discard = discard_l;

  if (!bfd_link_relocatable (&link_info))
    {
      /* We always need at least some relaxation to handle code alignment.  */
      if (RELAXATION_DISABLED_BY_USER)
	TARGET_ENABLE_RELAXATION;
      else
	ENABLE_RELAXATION;
    }

  link_info.relax_pass = 2;
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int need_layout = 0;

  /* Don't attempt to discard unused .eh_frame sections until the final link,
     as we can't reliably tell if they're used until after relaxation.  */
  if (!bfd_link_relocatable (&link_info))
    {
      need_layout = bfd_elf_discard_info (link_info.output_bfd, &link_info);
      if (need_layout < 0)
	{
	  einfo (_("%X%P: .eh_frame/.stab edit: %E\n"));
	  return;
	}
    }

  /* PR 27566, if the phase of data segment is exp_seg_relro_adjust,
     that means we are still adjusting the relro, and shouldn't do the
     relaxations at this stage.  Otherwise, we will get the symbol
     values beofore handling the relro, and may cause truncated fails
     when the relax range crossing the data segment.  One of the solution
     is to monitor the data segment phase while relaxing, to know whether
     the relro has been handled or not.

     I think we probably need to record more information about data
     segment or alignments in the future, to make sure it is safe
     to doing relaxations.  */
  enum phase_enum *phase = &(expld.dataseg.phase);
  bfd_elf${ELFSIZE}_riscv_set_data_segment_info (&link_info, (int *) phase);

  ldelf_map_segments (need_layout);
}

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */

static void
riscv_create_output_section_statements (void)
{
  /* See PR 22920 for an example of why this is necessary.  */
  if (strstr (bfd_get_target (link_info.output_bfd), "riscv") == NULL)
    {
      /* The RISC-V backend needs special fields in the output hash structure.
	 These will only be created if the output format is a RISC-V format,
	 hence we do not support linking and changing output formats at the
	 same time.  Use a link followed by objcopy to change output formats.  */
      einfo (_("%F%P: error: cannot change output format"
	       " whilst linking %s binaries\n"), "RISC-V");
      return;
    }

  riscv_elf${ELFSIZE}_set_options (&link_info, &params);
}

EOF

LDEMUL_BEFORE_ALLOCATION=riscv_elf_before_allocation
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=riscv_create_output_section_statements
