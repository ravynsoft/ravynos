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

fragment <<EOF

/* --- \begin{pdp11.em} */
#include "getopt.h"

static void
gld${EMULATION_NAME}_before_parse (void)
{
  ldfile_set_output_arch ("`echo ${ARCH}`", bfd_arch_unknown);
  /* for PDP11 Unix compatibility, default to --omagic */
  config.magic_demand_paged = false;
  config.text_read_only = false;
}

/* PDP11 specific options.  */
#define OPTION_IMAGIC 301

static void
gld${EMULATION_NAME}_add_options
  (int ns ATTRIBUTE_UNUSED,
   char **shortopts,
   int nl,
   struct option **longopts,
   int nrl ATTRIBUTE_UNUSED,
   struct option **really_longopts ATTRIBUTE_UNUSED)
{
  static const char xtra_short[] = "z";
  static const struct option xtra_long[] =
  {
    {"imagic", no_argument, NULL, OPTION_IMAGIC},
    {NULL, no_argument, NULL, 0}
  };

  *shortopts = (char *) xrealloc (*shortopts, ns + sizeof (xtra_short));
  memcpy (*shortopts + ns, &xtra_short, sizeof (xtra_short));
  *longopts
    = xrealloc (*longopts, nl * sizeof (struct option) + sizeof (xtra_long));
  memcpy (*longopts + nl, &xtra_long, sizeof (xtra_long));
}

static void
gld${EMULATION_NAME}_list_options (FILE *file)
{
  fprintf (file, _("  -N, --omagic   Do not make text readonly, do not page align data (default)\n"));
  fprintf (file, _("  -n, --nmagic   Make text readonly, align data to next page\n"));
  fprintf (file, _("  -z, --imagic   Make text readonly, separate instruction and data spaces\n"));
  fprintf (file, _("  --no-omagic    Equivalent to --nmagic\n"));
}

static bool
gld${EMULATION_NAME}_handle_option (int optc)
{
  switch (optc)
    {
    default:
      return false;

    case 'z':
    case OPTION_IMAGIC:
      link_info.separate_code = 1;
      /* The --imagic format causes the .text and .data sections to occupy the
	 same memory addresses in separate spaces, so don't check overlap. */
      command_line.check_section_addresses = 0;
      break;
    }

  return true;
}

/* We need a special case to prepare an additional linker script for option
 * --imagic where the .data section starts at address 0 rather than directly
 * following the .text section or being aligned to the next page after the
 * .text section. */
static char *
gld${EMULATION_NAME}_get_script (int *isfile)
EOF

if test x"$COMPILE_IN" = xyes
then
# Scripts compiled in.

# sed commands to quote an ld script as a C string.
sc="-f stringify.sed"

fragment <<EOF
{
  *isfile = 0;

  if (bfd_link_relocatable (&link_info) && config.build_constructors)
    return
EOF
sed $sc ldscripts/${EMULATION_NAME}.xu			>> e${EMULATION_NAME}.c
echo '  ; else if (bfd_link_relocatable (&link_info)) return' >> e${EMULATION_NAME}.c
sed $sc ldscripts/${EMULATION_NAME}.xr			>> e${EMULATION_NAME}.c
echo '  ; else if (link_info.separate_code) return'	>> e${EMULATION_NAME}.c
sed $sc ldscripts/${EMULATION_NAME}.xe			>> e${EMULATION_NAME}.c
echo '  ; else if (!config.text_read_only) return'	>> e${EMULATION_NAME}.c
sed $sc ldscripts/${EMULATION_NAME}.xbn			>> e${EMULATION_NAME}.c
echo '  ; else if (!config.magic_demand_paged) return'	>> e${EMULATION_NAME}.c
sed $sc ldscripts/${EMULATION_NAME}.xn			>> e${EMULATION_NAME}.c
echo '  ; else return'					>> e${EMULATION_NAME}.c
sed $sc ldscripts/${EMULATION_NAME}.x			>> e${EMULATION_NAME}.c
echo '; }'						>> e${EMULATION_NAME}.c

else
# Scripts read from the filesystem.

fragment <<EOF
{
  *isfile = 1;

  if (bfd_link_relocatable (&link_info) && config.build_constructors)
    return "ldscripts/${EMULATION_NAME}.xu";
  else if (bfd_link_relocatable (&link_info))
    return "ldscripts/${EMULATION_NAME}.xr";
  else if (link_info.separate_code)
    return "ldscripts/${EMULATION_NAME}.xe";
  else if (!config.text_read_only)
    return "ldscripts/${EMULATION_NAME}.xbn";
  else if (!config.magic_demand_paged)
    return "ldscripts/${EMULATION_NAME}.xn";
  else
    return "ldscripts/${EMULATION_NAME}.x";
}
EOF
fi

fragment <<EOF

/* --- \end{pdp11.em} */

EOF

LDEMUL_BEFORE_PARSE=gld"$EMULATION_NAME"_before_parse
LDEMUL_ADD_OPTIONS=gld"$EMULATION_NAME"_add_options
LDEMUL_HANDLE_OPTION=gld"$EMULATION_NAME"_handle_option
LDEMUL_LIST_OPTIONS=gld"$EMULATION_NAME"_list_options
LDEMUL_GET_SCRIPT=gld"$EMULATION_NAME"_get_script
