# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra C6X DSBT specific
# features.
#
fragment <<EOF
#include "ldctor.h"
#include "elf32-tic6x.h"

static struct elf32_tic6x_params params =
{
  0, 64
};

static int merge_exidx_entries = -1;

static int
is_tic6x_target (void)
{
  extern const bfd_target tic6x_elf32_le_vec;
  extern const bfd_target tic6x_elf32_be_vec;
  extern const bfd_target tic6x_elf32_linux_le_vec;
  extern const bfd_target tic6x_elf32_linux_be_vec;
  extern const bfd_target tic6x_elf32_c6000_le_vec;
  extern const bfd_target tic6x_elf32_c6000_be_vec;

  return (link_info.output_bfd->xvec == &tic6x_elf32_le_vec
	  || link_info.output_bfd->xvec == &tic6x_elf32_be_vec
	  || link_info.output_bfd->xvec == &tic6x_elf32_linux_le_vec
	  || link_info.output_bfd->xvec == &tic6x_elf32_linux_be_vec
	  || link_info.output_bfd->xvec == &tic6x_elf32_c6000_le_vec
	  || link_info.output_bfd->xvec == &tic6x_elf32_c6000_be_vec);
}

/* Pass params to backend.  */

static void
tic6x_after_open (void)
{
  if (is_tic6x_target ())
    {
      if (params.dsbt_index >= params.dsbt_size)
	{
	  einfo (_("%F%P: invalid --dsbt-index %d, outside DSBT size\n"),
		 params.dsbt_index);
	}
      elf32_tic6x_setup (&link_info, &params);
    }

  gld${EMULATION_NAME}_after_open ();
}

static int
compare_output_sec_vma (const void *a, const void *b)
{
  asection *asec = *(asection **) a, *bsec = *(asection **) b;
  asection *aout = asec->output_section, *bout = bsec->output_section;
  bfd_vma avma, bvma;

  /* If there's no output section for some reason, compare equal.  */
  if (!aout || !bout)
    return 0;

  avma = aout->vma + asec->output_offset;
  bvma = bout->vma + bsec->output_offset;

  if (avma > bvma)
    return 1;
  else if (avma < bvma)
    return -1;

  return 0;
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int layout_changed = 0;
  int ret;

  if (!bfd_link_relocatable (&link_info))
    {
      /* Build a sorted list of input text sections, then use that to process
	 the unwind table index.  */
      unsigned int list_size = 10;
      asection **sec_list = (asection **)
	xmalloc (list_size * sizeof (asection *));
      unsigned int sec_count = 0;

      LANG_FOR_EACH_INPUT_STATEMENT (is)
	{
	  bfd *abfd = is->the_bfd;
	  asection *sec;

	  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
	    continue;

	  for (sec = abfd->sections; sec != NULL; sec = sec->next)
	    {
	      asection *out_sec = sec->output_section;

	      if (out_sec
		  && elf_section_data (sec)
		  && elf_section_type (sec) == SHT_PROGBITS
		  && (elf_section_flags (sec) & SHF_EXECINSTR) != 0
		  && (sec->flags & SEC_EXCLUDE) == 0
		  && sec->sec_info_type != SEC_INFO_TYPE_JUST_SYMS
		  && out_sec != bfd_abs_section_ptr)
		{
		  if (sec_count == list_size)
		    {
		      list_size *= 2;
		      sec_list = (asection **)
			xrealloc (sec_list, list_size * sizeof (asection *));
		    }

		  sec_list[sec_count++] = sec;
		}
	    }
	}

      qsort (sec_list, sec_count, sizeof (asection *), &compare_output_sec_vma);

      if (elf32_tic6x_fix_exidx_coverage (sec_list, sec_count, &link_info,
					   merge_exidx_entries))
	layout_changed = 1;

      free (sec_list);
    }

  /* bfd_elf32_discard_info just plays with debugging sections,
     ie. doesn't affect any code, so we can delay resizing the
     sections.  */
  ret = bfd_elf_discard_info (link_info.output_bfd, & link_info);
  if (ret < 0)
    {
      einfo (_("%X%P: .eh_frame/.stab edit: %E\n"));
      return;
    }
  else if (ret > 0)
    layout_changed = 1;

  ldelf_map_segments (layout_changed);
}
EOF

# This code gets inserted into the generic elf32.sc linker script
# and allows us to define our own command line switches.
PARSE_AND_LIST_PROLOGUE='
#define OPTION_DSBT_INDEX		300
#define OPTION_DSBT_SIZE		301
#define OPTION_NO_MERGE_EXIDX_ENTRIES   302
'

PARSE_AND_LIST_LONGOPTS='
  {"dsbt-index", required_argument, NULL, OPTION_DSBT_INDEX},
  {"dsbt-size", required_argument, NULL, OPTION_DSBT_SIZE},
  { "no-merge-exidx-entries", no_argument, NULL, OPTION_NO_MERGE_EXIDX_ENTRIES },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --dsbt-index <index>    Use this as the DSBT index for the output object\n"));
  fprintf (file, _("  --dsbt-size <index>     Use this as the number of entries in the DSBT table\n"));
  fprintf (file, _("  --no-merge-exidx-entries\n"));
  fprintf (file, _("                          Disable merging exidx entries\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_DSBT_INDEX:
      {
	char *end;
	params.dsbt_index = strtol (optarg, &end, 0);
	if (*end == 0
	    && params.dsbt_index >= 0 && params.dsbt_index < 0x7fff)
	  break;
	einfo (_("%F%P: invalid --dsbt-index %s\n"), optarg);
      }
      break;
    case OPTION_DSBT_SIZE:
      {
	char *end;
	params.dsbt_size = strtol (optarg, &end, 0);
	if (*end == 0
	    && params.dsbt_size >= 0 && params.dsbt_size < 0x7fff)
	  break;
	einfo (_("%F%P: invalid --dsbt-size %s\n"), optarg);
      }
      break;
   case OPTION_NO_MERGE_EXIDX_ENTRIES:
      merge_exidx_entries = 0;
'

LDEMUL_AFTER_OPEN=tic6x_after_open
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
