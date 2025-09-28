# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2006-2023 Free Software Foundation, Inc.
#   Contributed by:
#   Brain.lin (brain.lin@sunplusct.com)
#   Mei Ligang (ligang@sunnorth.com.cn)
#   Pei-Lin Tsai (pltsai@sunplus.com)

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

# This file is sourced from elf.em, and defines extra score-elf
# specific routines.
#
fragment <<EOF

#include "elf32-score.h"

static void
gld${EMULATION_NAME}_before_parse (void)
{
#ifndef TARGET_			/* I.e., if not generic.  */
  ldfile_set_output_arch ("`echo ${ARCH}`", bfd_arch_unknown);
#endif /* not TARGET_ */
  input_flags.dynamic = ${DYNAMIC_LINK-true};
  config.has_shared = `if test -n "$GENERATE_SHLIB_SCRIPT" ; then echo true ; else echo false ; fi`;
  config.separate_code = `if test "x${SEPARATE_CODE}" = xyes ; then echo true ; else echo false ; fi`;
  link_info.check_relocs_after_open_input = true;
EOF
if test -n "$COMMONPAGESIZE"; then
fragment <<EOF
  link_info.relro = DEFAULT_LD_Z_RELRO;
EOF
fi
fragment <<EOF
  link_info.separate_code = DEFAULT_LD_Z_SEPARATE_CODE;
  link_info.warn_execstack = DEFAULT_LD_WARN_EXECSTACK;
  link_info.no_warn_rwx_segments = ! DEFAULT_LD_WARN_RWX_SEGMENTS;
  link_info.default_execstack = DEFAULT_LD_EXECSTACK;
}

static void
score_elf_after_open (void)
{
  if (strstr (bfd_get_target (link_info.output_bfd), "score") == NULL)
    {
      /* The score backend needs special fields in the output hash structure.
	 These will only be created if the output format is an score format,
	 hence we do not support linking and changing output formats at the
	 same time.  Use a link followed by objcopy to change output formats.  */
      einfo (_("%F%P: error: cannot change output format "
	       "whilst linking %s binaries\n"), "S+core");
      return;
    }

  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_open ();
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE=''
PARSE_AND_LIST_SHORTOPTS=
PARSE_AND_LIST_LONGOPTS=''
PARSE_AND_LIST_OPTIONS=''
PARSE_AND_LIST_ARGS_CASES=''

# We have our own after_open and before_allocation functions, but they call
# the standard routines, so give them a different name.
LDEMUL_AFTER_OPEN=score_elf_after_open

# Replace the elf before_parse function with our own.
LDEMUL_BEFORE_PARSE=gld"${EMULATION_NAME}"_before_parse
