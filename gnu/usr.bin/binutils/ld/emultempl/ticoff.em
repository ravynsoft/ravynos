# This shell script emits a C file. -*- C -*-
# It does some substitutions.
(echo;echo;echo;echo)>e${EMULATION_NAME}.c # there, now line numbers match ;-)
fragment <<EOF
/* This file is part of GLD, the Gnu Linker.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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

/* For TI COFF */
/* Need to determine load and run pages for output sections */

#define TARGET_IS_${EMULATION_NAME}

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ctf-api.h"
#include "getopt.h"

#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"

static int coff_version;

/* TI COFF extra command line options */
#define OPTION_COFF_FORMAT		(300 + 1)

static void
gld${EMULATION_NAME}_add_options
  (int ns ATTRIBUTE_UNUSED, char **shortopts ATTRIBUTE_UNUSED, int nl,
   struct option **longopts, int nrl ATTRIBUTE_UNUSED,
   struct option **really_longopts ATTRIBUTE_UNUSED)
{
  static const struct option xtra_long[] = {
    /* TI COFF options */
    {"format", required_argument, NULL, OPTION_COFF_FORMAT },
    {NULL, no_argument, NULL, 0}
  };

  *longopts = (struct option *)
    xrealloc (*longopts, nl * sizeof (struct option) + sizeof (xtra_long));
  memcpy (*longopts + nl, &xtra_long, sizeof (xtra_long));
}

static void
gld${EMULATION_NAME}_list_options (FILE * file)
{
  fprintf (file, _("  --format 0|1|2              Specify which COFF version to use\n"));
}

static bool
gld${EMULATION_NAME}_handle_option (int optc)
{
  switch (optc)
    {
    default:
      return false;

    case OPTION_COFF_FORMAT:
      if ((*optarg == '0' || *optarg == '1' || *optarg == '2')
	  && optarg[1] == '\0')
	{
	  static char buf[] = "coffX-${OUTPUT_FORMAT_TEMPLATE}";
	  coff_version = *optarg - '0';
	  buf[4] = *optarg;
	  lang_add_output_format (buf, NULL, NULL, 0);
	}
      else
	{
	  einfo (_("%F%P: invalid COFF format version %s\n"), optarg);
	}
      break;
    }
  return false;
}

static void
gld${EMULATION_NAME}_before_parse(void)
{
#ifndef TARGET_			/* I.e., if not generic.  */
  ldfile_set_output_arch ("`echo ${ARCH}`", bfd_arch_unknown);
#endif /* not TARGET_ */
}

static char *
gld${EMULATION_NAME}_get_script (int *isfile)
EOF
if test x"$COMPILE_IN" = xyes
then
# Scripts compiled in.

# sed commands to quote an ld script as a C string.
sc='s/["\\]/\\&/g
s/$/\\n\\/
1s/^/"/
$s/$/n"/
'
fragment <<EOF
{
  *isfile = 0;
  if (bfd_link_relocatable (&link_info) && config.build_constructors)
    return `sed "$sc" ldscripts/${EMULATION_NAME}.xu`;
  else if (bfd_link_relocatable (&link_info))
    return `sed "$sc" ldscripts/${EMULATION_NAME}.xr`;
  else if (!config.text_read_only)
    return `sed "$sc" ldscripts/${EMULATION_NAME}.xbn`;
  else if (!config.magic_demand_paged)
    return `sed "$sc" ldscripts/${EMULATION_NAME}.xn`;
  else
    return `sed "$sc" ldscripts/${EMULATION_NAME}.x`;
}
EOF

else
# Scripts read from the filesystem.

fragment <<EOF
{
  *isfile = 1;

  if (bfd_link_relocatable (&link_info) && config.build_constructors)
    return "ldscripts/${EMULATION_NAME}.xu";
  else if (bfd_link_relocatable (&link_info))
    return "ldscripts/${EMULATION_NAME}.xr";
  else if (!config.text_read_only)
    return "ldscripts/${EMULATION_NAME}.xbn";
  else if (!config.magic_demand_paged)
    return "ldscripts/${EMULATION_NAME}.xn";
  else
    return "ldscripts/${EMULATION_NAME}.x";
}
EOF

fi

LDEMUL_ADD_OPTIONS=gld${EMULATION_NAME}_add_options
LDEMUL_HANDLE_OPTION=gld${EMULATION_NAME}_handle_option
LDEMUL_LIST_OPTIONS=gld${EMULATION_NAME}_list_options

source_em ${srcdir}/emultempl/emulation.em
