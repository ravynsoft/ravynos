# This shell script emits a C file. -*- C -*-
#   Copyright (C) 1991-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra hppa-elf
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf32-hppa.h"


/* Fake input file for stubs.  */
static lang_input_statement_type *stub_file;

/* Type of import/export stubs to build.  For a single sub-space model,
   we can build smaller import stubs and there is no need for export
   stubs.  */
static int multi_subspace = 0;

/* Whether we need to call hppa_layout_sections_again.  */
static int need_laying_out = 0;

/* Maximum size of a group of input sections that can be handled by
   one stub section.  A value of +/-1 indicates the bfd back-end
   should use a suitable default size.  */
static bfd_signed_vma group_size = 1;

/* Stops the linker merging .text sections on a relocatable link,
   and adds millicode library to the list of input files.  */

static void
hppaelf_after_parse (void)
{
  if (bfd_link_relocatable (&link_info))
    lang_add_unique (".text");

  /* Enable this once we split millicode stuff from libgcc:
     lang_add_input_file ("milli",
			  lang_input_file_is_l_enum,
			  NULL);
  */

  ldelf_after_parse ();
}

/* This is called before the input files are opened.  We create a new
   fake input file to hold the stub sections.  */

static void
hppaelf_create_output_section_statements (void)
{
  if (!(bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
	&& (elf_object_id (link_info.output_bfd) == HPPA32_ELF_DATA
	    || elf_object_id (link_info.output_bfd) == HPPA64_ELF_DATA)))
    return;

  stub_file = lang_add_input_file ("linker stubs",
				   lang_input_file_is_fake_enum,
				   NULL);
  stub_file->the_bfd = bfd_create ("linker stubs", link_info.output_bfd);
  if (stub_file->the_bfd == NULL
      || ! bfd_set_arch_mach (stub_file->the_bfd,
			      bfd_get_arch (link_info.output_bfd),
			      bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%F%P: can not create BFD: %E\n"));
      return;
    }

  stub_file->the_bfd->flags |= BFD_LINKER_CREATED;
  ldlang_add_file (stub_file);
  elf32_hppa_init_stub_bfd (stub_file->the_bfd, &link_info);
}


struct hook_stub_info
{
  lang_statement_list_type add;
  asection *input_section;
};

/* Traverse the linker tree to find the spot where the stub goes.  */

static bool
hook_in_stub (struct hook_stub_info *info, lang_statement_union_type **lp)
{
  lang_statement_union_type *l;
  bool ret;

  for (; (l = *lp) != NULL; lp = &l->header.next)
    {
      switch (l->header.type)
	{
	case lang_constructors_statement_enum:
	  ret = hook_in_stub (info, &constructor_list.head);
	  if (ret)
	    return ret;
	  break;

	case lang_output_section_statement_enum:
	  ret = hook_in_stub (info,
			      &l->output_section_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_wild_statement_enum:
	  ret = hook_in_stub (info, &l->wild_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_group_statement_enum:
	  ret = hook_in_stub (info, &l->group_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_input_section_enum:
	  if (l->input_section.section == info->input_section)
	    {
	      /* We've found our section.  Insert the stub immediately
		 before its associated input section.  */
	      *lp = info->add.head;
	      *(info->add.tail) = l;
	      return true;
	    }
	  break;

	case lang_data_statement_enum:
	case lang_reloc_statement_enum:
	case lang_object_symbols_statement_enum:
	case lang_output_statement_enum:
	case lang_target_statement_enum:
	case lang_input_statement_enum:
	case lang_assignment_statement_enum:
	case lang_padding_statement_enum:
	case lang_address_statement_enum:
	case lang_fill_statement_enum:
	  break;

	default:
	  FAIL ();
	  break;
	}
    }
  return false;
}


/* Call-back for elf32_hppa_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately before INPUT_SECTION.  */

static asection *
hppaelf_add_stub_section (const char *stub_sec_name, asection *input_section)
{
  asection *stub_sec;
  flagword flags;
  asection *output_section;
  lang_output_section_statement_type *os;
  struct hook_stub_info info;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
	   | SEC_HAS_CONTENTS | SEC_RELOC | SEC_IN_MEMORY | SEC_KEEP);
  stub_sec = bfd_make_section_anyway_with_flags (stub_file->the_bfd,
						 stub_sec_name, flags);
  if (stub_sec == NULL)
    goto err_ret;

  output_section = input_section->output_section;
  os = lang_output_section_get (output_section);

  info.input_section = input_section;
  lang_list_init (&info.add);
  lang_add_section (&info.add, stub_sec, NULL, NULL, os);

  if (info.add.head == NULL)
    goto err_ret;

  if (hook_in_stub (&info, &os->children.head))
    return stub_sec;

 err_ret:
  einfo (_("%X%P: can not make stub section: %E\n"));
  return NULL;
}


/* Another call-back for elf32_hppa_size_stubs.  */

static void
hppaelf_layout_sections_again (void)
{
  /* If we have changed sizes of the stub sections, then we need
     to recalculate all the section offsets.  This may mean we need to
     add even more stubs.  */
  ldelf_map_segments (true);
  need_laying_out = -1;
}


static void
build_section_lists (lang_statement_union_type *statement)
{
  if (statement->header.type == lang_input_section_enum)
    {
      asection *i = statement->input_section.section;

      if (i->sec_info_type != SEC_INFO_TYPE_JUST_SYMS
	  && (i->flags & SEC_EXCLUDE) == 0
	  && i->output_section != NULL
	  && i->output_section->owner == link_info.output_bfd)
	{
	  elf32_hppa_next_input_section (&link_info, i);
	}
    }
}


/* For the PA we use this opportunity to size and build linker stubs.  */

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int ret;

  /* bfd_elf_discard_info just plays with data and debugging sections,
     ie. doesn't affect code size, so we can delay resizing the
     sections.  It's likely we'll resize everything in the process of
     adding stubs.  */
  ret = bfd_elf_discard_info (link_info.output_bfd, &link_info);
  if (ret < 0)
    {
      einfo (_("%X%P: .eh_frame/.stab edit: %E\n"));
      return;
    }
  else if (ret > 0)
    need_laying_out = 1;

  /* If generating a relocatable output file, then we don't
     have to examine the relocs.  */
  if (stub_file != NULL && !bfd_link_relocatable (&link_info))
    {
      ret = elf32_hppa_setup_section_lists (link_info.output_bfd, &link_info);
      if (ret != 0)
	{
	  if (ret < 0)
	    {
	      einfo (_("%X%P: can not size stub section: %E\n"));
	      return;
	    }

	  lang_for_each_statement (build_section_lists);

	  /* Call into the BFD backend to do the real work.  */
	  if (! elf32_hppa_size_stubs (link_info.output_bfd,
				       stub_file->the_bfd,
				       &link_info,
				       multi_subspace,
				       group_size,
				       &hppaelf_add_stub_section,
				       &hppaelf_layout_sections_again))
	    {
	      einfo (_("%X%P: can not size stub section: %E\n"));
	      return;
	    }
	}
    }

  if (need_laying_out != -1)
    ldelf_map_segments (need_laying_out);

  if (!bfd_link_relocatable (&link_info))
    {
      /* Set the global data pointer.  */
      if (! elf32_hppa_set_gp (link_info.output_bfd, &link_info))
	{
	  einfo (_("%X%P: can not set gp\n"));
	  return;
	}

      /* Now build the linker stubs.  */
      if (stub_file != NULL && stub_file->the_bfd->sections != NULL)
	{
	  if (! elf32_hppa_build_stubs (&link_info))
	    einfo (_("%X%P: can not build stubs: %E\n"));
	}
    }
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_MULTI_SUBSPACE		301
#define OPTION_STUBGROUP_SIZE		(OPTION_MULTI_SUBSPACE + 1)
'

PARSE_AND_LIST_LONGOPTS='
  { "multi-subspace", no_argument, NULL, OPTION_MULTI_SUBSPACE },
  { "stub-group-size", required_argument, NULL, OPTION_STUBGROUP_SIZE },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("\
  --multi-subspace            Generate import and export stubs to support\n\
                                multiple sub-space shared libraries\n"
		   ));
  fprintf (file, _("\
  --stub-group-size=N         Maximum size of a group of input sections that\n\
                                can be handled by one stub section.  A negative\n\
                                value locates all stubs before their branches\n\
                                (with a group size of -N), while a positive\n\
                                value allows two groups of input sections, one\n\
                                before, and one after each stub section.\n\
                                Values of +/-1 indicate the linker should\n\
                                choose suitable defaults.\n"
		   ));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_MULTI_SUBSPACE:
      multi_subspace = 1;
      break;

    case OPTION_STUBGROUP_SIZE:
      {
	const char *end;
	group_size = bfd_scan_vma (optarg, &end, 0);
	if (*end)
	  einfo (_("%F%P: invalid number `%s'\''\n"), optarg);
      }
      break;
'

# Put these extra hppaelf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_AFTER_PARSE=hppaelf_after_parse
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=hppaelf_create_output_section_statements
