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

if test -n "$VXWORKS_BASE_EM_FILE" ; then
  source_em "${srcdir}/emultempl/${VXWORKS_BASE_EM_FILE}.em"
fi

fragment <<EOF

static int force_dynamic;

static void
vxworks_before_parse (void)
{
  ${LDEMUL_BEFORE_PARSE-gld${EMULATION_NAME}_before_parse} ();
  config.rpath_separator = ';';
}

static void
vxworks_after_open (void)
{
  ${LDEMUL_AFTER_OPEN-gld${EMULATION_NAME}_after_open} ();

  if (force_dynamic
      && link_info.input_bfds
      && bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && !_bfd_elf_link_create_dynamic_sections (link_info.input_bfds,
						 &link_info))
    einfo (_("%X%P: cannot create dynamic sections %E\n"));

  if (!force_dynamic
      && !bfd_link_pic (&link_info)
      && bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && elf_hash_table (&link_info)->dynamic_sections_created)
    einfo (_("%X%P: dynamic sections created in non-dynamic link\n"));
}

EOF

PARSE_AND_LIST_PROLOGUE=$PARSE_AND_LIST_PROLOGUE'
enum {
  OPTION_FORCE_DYNAMIC = 501
};
'

PARSE_AND_LIST_LONGOPTS=$PARSE_AND_LIST_LONGOPTS'
  {"force-dynamic", no_argument, NULL, OPTION_FORCE_DYNAMIC},
'

PARSE_AND_LIST_OPTIONS=$PARSE_AND_LIST_OPTIONS'
  fprintf (file, _("\
  --force-dynamic             Always create dynamic sections\n"));
'

PARSE_AND_LIST_ARGS_CASES=$PARSE_AND_LIST_ARGS_CASES'
    case OPTION_FORCE_DYNAMIC:
      force_dynamic = 1;
      break;
'

# Hook in our routines above.  There are three possibilities:
#
#   (1) VXWORKS_BASE_EM_FILE did not set the hook's LDEMUL_FOO variable.
#	We want to define LDEMUL_FOO to vxworks_foo in that case,
#
#   (2) VXWORKS_BASE_EM_FILE set the hook's LDEMUL_FOO variable to
#	gld${EMULATION_NAME}_foo.  This means that the file has
#	replaced elf.em's default definition, so we simply #define
#	the current value of LDEMUL_FOO to vxworks_foo.
#
#   (3) VXWORKS_BASE_EM_FILE set the hook's LDEMUL_FOO variable to
#	something other than gld${EMULATION_NAME}_foo.  We handle
#	this case in the same way as (1).
for override in before_parse after_open; do
  var="LDEMUL_`echo ${override} | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`"
  eval value=\$${var}
  if test "${value}" = "gld${EMULATION_NAME}_${override}"; then
    fragment <<EOF
#define ${value} vxworks_${override}
EOF
  else
    eval $var=vxworks_${override}
  fi
done
