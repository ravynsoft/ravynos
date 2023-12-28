# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# This file is sourced from generic.em.

fragment <<EOF
#include "getopt.h"

static void
gld${EMULATION_NAME}_before_parse (void)
{
  ldfile_set_output_arch ("${ARCH}", bfd_arch_`echo ${ARCH} | sed -e 's/:.*//'`);
  input_flags.dynamic = true;
  config.has_shared = false; /* Not yet.  */

  /* For ia64, harmless for alpha.  */
  link_info.emit_hash = false;
  link_info.spare_dynamic_tags = 0;
}

/* This is called before the input files are opened.  We add the
   standard library.  */

static void
gld${EMULATION_NAME}_create_output_section_statements (void)
{
  lang_add_input_file ("imagelib", lang_input_file_is_l_enum, NULL);
  lang_add_input_file ("starlet", lang_input_file_is_l_enum, NULL);
  lang_add_input_file ("sys\$public_vectors", lang_input_file_is_l_enum, NULL);
}

/* Try to open a dynamic archive.  This is where we know that VMS
   shared images (dynamic libraries) have an extension of .exe.  */

static bool
gld${EMULATION_NAME}_open_dynamic_archive (const char *arch ATTRIBUTE_UNUSED,
					   search_dirs_type *search,
					   lang_input_statement_type *entry)
{
  char *string;

  if (! entry->flags.maybe_archive || entry->flags.full_name_provided)
    return false;

  string = (char *) xmalloc (strlen (search->name)
			     + strlen (entry->filename)
			     + sizeof "/.exe");

  sprintf (string, "%s/%s.exe", search->name, entry->filename);

  if (! ldfile_try_open_bfd (string, entry))
    {
      free (string);
      return false;
    }

  entry->filename = string;

  return true;
}

static int
gld${EMULATION_NAME}_find_potential_libraries
  (char *name, lang_input_statement_type *entry)
{
  return ldfile_open_file_search (name, entry, "", ".olb");
}

/* Place an orphan section.  We use this to put random OVR sections.
   Much borrowed from elf.em.  */

static lang_output_section_statement_type *
vms_place_orphan (asection *s,
		  const char *secname ATTRIBUTE_UNUSED,
		  int constraint ATTRIBUTE_UNUSED)
{
  static struct orphan_save hold_data =
    {
      "\$DATA\$",
      SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_DATA,
      0, 0, 0, 0
    };

  /* We have nothing to say for anything other than a final link or an excluded
     section.  */
  if (bfd_link_relocatable (&link_info)
      || (s->flags & (SEC_EXCLUDE | SEC_LOAD)) != SEC_LOAD)
    return NULL;

  /* FIXME: we should place sections by VMS program section flags.  */

  /* Only handle data sections.  */
  if ((s->flags & SEC_DATA) == 0)
    return NULL;

  if (hold_data.os == NULL)
    hold_data.os = lang_output_section_find (hold_data.name);

  if (hold_data.os != NULL)
    {
      lang_add_section (&hold_data.os->children, s, NULL, NULL, hold_data.os);
      return hold_data.os;
    }
  else
    return NULL;
}

/* VMS specific options.  */
#define OPTION_IDENTIFICATION		(300  + 1)

static void
gld${EMULATION_NAME}_add_options
  (int ns ATTRIBUTE_UNUSED,
   char **shortopts ATTRIBUTE_UNUSED,
   int nl,
   struct option **longopts,
   int nrl ATTRIBUTE_UNUSED,
   struct option **really_longopts ATTRIBUTE_UNUSED)
{
  static const struct option xtra_long[] =
  {
    {"identification", required_argument, NULL, OPTION_IDENTIFICATION},
    {NULL, no_argument, NULL, 0}
  };

  *longopts
    = xrealloc (*longopts, nl * sizeof (struct option) + sizeof (xtra_long));
  memcpy (*longopts + nl, &xtra_long, sizeof (xtra_long));
}

static void
gld${EMULATION_NAME}_list_options (FILE *file)
{
  fprintf (file, _("  --identification <string>          Set the identification of the output\n"));
}

static bool
gld${EMULATION_NAME}_handle_option (int optc)
{
  switch (optc)
    {
    default:
      return false;

    case OPTION_IDENTIFICATION:
      /* Currently ignored.  */
      break;
    }

  return true;
}

EOF

if test "$OUTPUT_FORMAT" = "elf64-ia64-vms"; then

fragment <<EOF
#include "elf-bfd.h"
#include "ldelfgen.h"
EOF

source_em ${srcdir}/emultempl/elf-generic.em

fragment <<EOF

/* This is called after the sections have been attached to output
   sections, but before any sizes or addresses have been set.  */

static void
gld${EMULATION_NAME}_before_allocation (void)
{
  const struct elf_backend_data *bed;

  if (!is_elf_hash_table (link_info.hash))
    return;

  bed = get_elf_backend_data (link_info.output_bfd);

  /* The backend must work out the sizes of all the other dynamic
     sections.  */
  if (elf_hash_table (&link_info)->dynamic_sections_created
      && bed->elf_backend_size_dynamic_sections
      && ! (*bed->elf_backend_size_dynamic_sections) (link_info.output_bfd,
						      &link_info))
    einfo (_("%F%P: failed to set dynamic section sizes: %E\n"));

  before_allocation_default ();
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int need_layout = bfd_elf_discard_info (link_info.output_bfd, &link_info);

  if (need_layout < 0)
    einfo (_("%X%P: .eh_frame/.stab edit: %E\n"));
  else
    ldelf_map_segments (need_layout);
}

static void
gld${EMULATION_NAME}_after_parse (void)
{
  link_info.relax_pass = 2;
  after_parse_default ();
}
EOF

LDEMUL_BEFORE_ALLOCATION=gld"$EMULATION_NAME"_before_allocation
LDEMUL_AFTER_ALLOCATION=gld"$EMULATION_NAME"_after_allocation

LDEMUL_AFTER_PARSE=gld${EMULATION_NAME}_after_parse
source_em ${srcdir}/emultempl/needrelax.em
fi

LDEMUL_PLACE_ORPHAN=vms_place_orphan
LDEMUL_BEFORE_PARSE=gld"$EMULATION_NAME"_before_parse
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=gld"$EMULATION_NAME"_create_output_section_statements
LDEMUL_FIND_POTENTIAL_LIBRARIES=gld"$EMULATION_NAME"_find_potential_libraries
LDEMUL_OPEN_DYNAMIC_ARCHIVE=gld"$EMULATION_NAME"_open_dynamic_archive
LDEMUL_ADD_OPTIONS=gld"$EMULATION_NAME"_add_options
LDEMUL_HANDLE_OPTION=gld"$EMULATION_NAME"_handle_option
LDEMUL_LIST_OPTIONS=gld"$EMULATION_NAME"_list_options
