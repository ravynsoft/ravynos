# This shell script emits a C file. -*- C -*-
# It does some substitutions.
fragment <<EOF
/* A vanilla emulation with no defaults
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain steve@cygnus.com

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ctf-api.h"

#include "ld.h"
#include "ldmisc.h"
#include "ldmain.h"

#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"

static void vanilla_before_parse (void)
{
}

static void
vanilla_set_output_arch (void)
{
  /* Set the output architecture and machine if possible */
  unsigned long  machine = 0;
  bfd_set_arch_mach (link_info.output_bfd,
		     ldfile_output_architecture, machine);
}

static char *
vanilla_get_script (int *isfile)
{
  *isfile = 0;
  return "";
}
EOF

LDEMUL_BEFORE_PARSE=vanilla_before_parse
LDEMUL_SET_OUTPUT_ARCH=vanilla_set_output_arch
LDEMUL_GET_SCRIPT=vanilla_get_script
EMULATION_NAME=vanilla
OUTPUT_FORMAT=a.out-sunos-big

source_em ${srcdir}/emultempl/emulation.em
