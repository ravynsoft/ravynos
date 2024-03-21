# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2013-2023 Free Software Foundation, Inc.
#
# This file is part of GNU Binutils.
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

# This file is sourced from elf.em, and defines extra C-SKY ELF
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf/csky.h"
#include "elf32-csky.h"

/* To use branch stub or not.  */
extern bool use_branch_stub;

/* Fake input file for stubs.  */
static lang_input_statement_type *stub_file;

/* Whether we need to call gldcsky_layout_sections_again.  */
static int need_laying_out = 0;

/* Maximum size of a group of input sections that can be handled by
   one stub section.  A value of +/-1 indicates the bfd back-end
   should use a suitable default size.  */
static bfd_signed_vma group_size = 1;

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

  for (l = *lp; l != NULL; lp = &l->header.next, l = *lp)
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
	       after its associated input section.  */
	    *(info->add.tail) = l->header.next;
	    l->header.next = info->add.head;
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

  return false;
}
EOF

case ${target} in
    csky-*-linux-*)
fragment <<EOF

static void
csky_elf_before_parse (void)
{
  use_branch_stub = false;
  gld${EMULATION_NAME}_before_parse ();
}
EOF
    ;;
esac

fragment <<EOF

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
csky_elf_create_output_section_statements (void)
{
  if (!(bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
	&& elf_object_id (link_info.output_bfd) == CSKY_ELF_DATA))
    use_branch_stub = false;

  /* If don't use branch stub, just do not emit stub_file.  */
  if (!use_branch_stub)
    return;

  stub_file = lang_add_input_file ("linker stubs",
				   lang_input_file_is_fake_enum, NULL);
  stub_file->the_bfd = bfd_create ("linker stubs", link_info.output_bfd);
  if (stub_file->the_bfd == NULL
      || !bfd_set_arch_mach (stub_file->the_bfd,
	  bfd_get_arch (link_info.output_bfd),
	  bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%F%P: can not create BFD: %E\n"));
      return;
    }

  stub_file->the_bfd->flags |= BFD_LINKER_CREATED;
  ldlang_add_file (stub_file);
}

/* Call-back for elf32_csky_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately after INPUT_SECTION.  */
static asection *
elf32_csky_add_stub_section (const char *stub_sec_name,
			     asection *input_section)
{
  asection *stub_sec;
  flagword flags;
  asection *output_section;
  const char *secname;
  lang_output_section_statement_type *os;
  struct hook_stub_info info;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
	   | SEC_HAS_CONTENTS | SEC_RELOC | SEC_IN_MEMORY | SEC_KEEP);
  stub_sec = bfd_make_section_anyway_with_flags (stub_file->the_bfd,
						 stub_sec_name, flags);
  if (stub_sec == NULL)
    goto err_ret;

  bfd_set_section_alignment (stub_sec, 3);

  output_section = input_section->output_section;
  secname = bfd_section_name (output_section);
  os = lang_output_section_find (secname);

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

/* Another call-back for elf_csky_size_stubs.  */
static void
gldcsky_layout_sections_again (void)
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
	elf32_csky_next_input_section (&link_info, i);
    }
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  /* bfd_elf32_discard_info just plays with debugging sections,
     ie. doesn't affect any code, so we can delay resizing the
     sections.  It's likely we'll resize everything in the process of
     adding stubs.  */
  if (bfd_elf_discard_info (link_info.output_bfd, &link_info))
    need_laying_out = 1;

  /* If generating a relocatable output file, then we don't
     have to examine the relocs.  */

  if (stub_file != NULL && !bfd_link_relocatable (&link_info))
    {
      int ret = elf32_csky_setup_section_lists (link_info.output_bfd,
						&link_info);

      if (ret < 0)
	{
	  einfo (_("%X%P: could not compute sections lists for stub generation: %E\n"));
	  return;
	}
      else if (ret != 0)
	{
	  lang_for_each_statement (build_section_lists);

	  /* Call into the BFD backend to do the real work.  */
	  if (! elf32_csky_size_stubs (link_info.output_bfd,
				       stub_file->the_bfd,
				       &link_info,
				       group_size,
				       &elf32_csky_add_stub_section,
				       &gldcsky_layout_sections_again))
	  {
	    einfo (_("%X%P: cannot size stub section: %E\n"));
	    return;
	  }
	}
    }

  if (need_laying_out != -1)
    ldelf_map_segments (need_laying_out);
}

static void
gld${EMULATION_NAME}_finish (void)
{
  if (stub_file != NULL
      && !bfd_link_relocatable (&link_info)
      && stub_file->the_bfd->sections != NULL
      && !elf32_csky_build_stubs (&link_info))
    einfo (_("%X%P: cannot build stubs: %E\n"));
  finish_default ();
}

EOF

# This code gets inserted into the generic elf32.sc linker script
# and allows us to define our own command line switches.
PARSE_AND_LIST_PROLOGUE='
#define OPTION_BRANCH_STUB		301
#define OPTION_NO_BRANCH_STUB		302
#define OPTION_STUBGROUP_SIZE		303
'

PARSE_AND_LIST_LONGOPTS='
  {"branch-stub",	no_argument,       NULL, OPTION_BRANCH_STUB},
  {"no-branch-stub",	no_argument,       NULL, OPTION_NO_BRANCH_STUB},
  {"stub-group-size",	required_argument, NULL, OPTION_STUBGROUP_SIZE},
'
PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --[no-]branch-stub          "
		   "Disable/enable use of stubs to expand branch\n"
		   "                              "
		   "  instructions that cannot reach the target.\n"));
  fprintf (file, _("  --stub-group-size=N         "
		   "Maximum size of a group of input sections\n"
		   "                              "
		   "  handled by one stub section.\n"));
'

PARSE_AND_LIST_ARGS_CASES='
  case OPTION_BRANCH_STUB:
    use_branch_stub = true;
    break;
  case OPTION_NO_BRANCH_STUB:
    use_branch_stub = false;
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

case ${target} in
    csky-*-linux-*) LDEMUL_BEFORE_PARSE=csky_elf_before_parse ;;
esac
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=csky_elf_create_output_section_statements
LDEMUL_FINISH=gld${EMULATION_NAME}_finish
