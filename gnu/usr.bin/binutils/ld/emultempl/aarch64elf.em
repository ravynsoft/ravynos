# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2009-2023 Free Software Foundation, Inc.
#   Contributed by ARM Ltd.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the license, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3. If not,
# see <http://www.gnu.org/licenses/>.
#

# This file is sourced from elf.em, and defines extra aarch64-elf
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf/aarch64.h"
#include "elfxx-aarch64.h"

static int no_enum_size_warning = 0;
static int no_wchar_size_warning = 0;
static int pic_veneer = 0;
static int fix_erratum_835769 = 0;
static erratum_84319_opts fix_erratum_843419 = ERRAT_NONE;
static int no_apply_dynamic_relocs = 0;
static aarch64_plt_type plt_type = PLT_NORMAL;
static aarch64_enable_bti_type bti_type = BTI_NONE;

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
aarch64_elf_before_allocation (void)
{
  /* We should be able to set the size of the interworking stub section.  We
     can't do it until later if we have dynamic sections, though.  */
  if (! elf_hash_table (&link_info)->dynamic_sections_created)
    {
      /* Here we rummage through the found bfds to collect information.  */
      LANG_FOR_EACH_INPUT_STATEMENT (is)
      {
	/* Initialise mapping tables for code/data.  */
	bfd_elf${ELFSIZE}_aarch64_init_maps (is->the_bfd);
      }
    }

  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_before_allocation ();
}

/* Fake input file for stubs.  */
static lang_input_statement_type *stub_file;

/* Whether we need to call gldarm_layout_sections_again.  */
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
    }
  return false;
}


/* Call-back for elf${ELFSIZE}_aarch64_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately after INPUT_SECTION.  */

static asection *
elf${ELFSIZE}_aarch64_add_stub_section (const char *stub_sec_name,
					asection *input_section)
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

  /* Long branch stubs contain a 64-bit address, so the section requires
     8 byte alignment.  */
  bfd_set_section_alignment (stub_sec, 3);

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

/* Another call-back for elf${ELFSIZE}_aarch64_size_stubs.  */

static void
gldaarch64_layout_sections_again (void)
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

      if (!bfd_input_just_syms (i->owner)
	  && (i->flags & SEC_EXCLUDE) == 0
	  && i->output_section != NULL
	  && i->output_section->owner == link_info.output_bfd)
	elf${ELFSIZE}_aarch64_next_input_section (& link_info, i);
    }
}

static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int ret;

  /* bfd_elf32_discard_info just plays with debugging sections,
     ie. doesn't affect any code, so we can delay resizing the
     sections.  It's likely we'll resize everything in the process of
     adding stubs.  */
  ret = bfd_elf_discard_info (link_info.output_bfd, & link_info);
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
      ret = elf${ELFSIZE}_aarch64_setup_section_lists (link_info.output_bfd,
						       &link_info);
      if (ret != 0)
	{
	  if (ret < 0)
	    {
	      einfo (_("%X%P: could not compute sections lists "
		       "for stub generation: %E\n"));
	      return;
	    }

	  lang_for_each_statement (build_section_lists);

	  /* Call into the BFD backend to do the real work.  */
	  if (! elf${ELFSIZE}_aarch64_size_stubs (link_info.output_bfd,
					  stub_file->the_bfd,
					  & link_info,
					  group_size,
					  & elf${ELFSIZE}_aarch64_add_stub_section,
					  & gldaarch64_layout_sections_again))
	    {
	      einfo (_("%X%P: can not size stub section: %E\n"));
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
  if (!bfd_link_relocatable (&link_info))
    {
      /* Now build the linker stubs.  */
      if (stub_file->the_bfd->sections != NULL)
	{
	  if (! elf${ELFSIZE}_aarch64_build_stubs (& link_info))
	    einfo (_("%X%P: can not build stubs: %E\n"));
	}
    }

  finish_default ();
}

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
aarch64_elf_create_output_section_statements (void)
{
  if (strstr (bfd_get_target (link_info.output_bfd), "aarch64") == NULL)
    {
      /* The arm backend needs special fields in the output hash structure.
	 These will only be created if the output format is an arm format,
	 hence we do not support linking and changing output formats at the
	 same time.  Use a link followed by objcopy to change output formats.  */
      einfo (_("%F%P: error: cannot change output format "
	       "whilst linking %s binaries\n"), "AArch64");
      return;
    }

  aarch64_bti_pac_info bp_info;
  bp_info.plt_type = plt_type;
  bp_info.bti_type = bti_type;

  bfd_elf${ELFSIZE}_aarch64_set_options (link_info.output_bfd, &link_info,
				 no_enum_size_warning,
				 no_wchar_size_warning,
				 pic_veneer,
				 fix_erratum_835769, fix_erratum_843419,
				 no_apply_dynamic_relocs,
				 bp_info);

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
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_NO_ENUM_SIZE_WARNING	309
#define OPTION_PIC_VENEER		310
#define OPTION_STUBGROUP_SIZE		311
#define OPTION_NO_WCHAR_SIZE_WARNING	312
#define OPTION_FIX_ERRATUM_835769	313
#define OPTION_FIX_ERRATUM_843419	314
#define OPTION_NO_APPLY_DYNAMIC_RELOCS	315
'

PARSE_AND_LIST_SHORTOPTS=p

PARSE_AND_LIST_LONGOPTS='
  { "no-pipeline-knowledge", no_argument, NULL, '\'p\''},
  { "no-enum-size-warning", no_argument, NULL, OPTION_NO_ENUM_SIZE_WARNING},
  { "pic-veneer", no_argument, NULL, OPTION_PIC_VENEER},
  { "stub-group-size", required_argument, NULL, OPTION_STUBGROUP_SIZE },
  { "no-wchar-size-warning", no_argument, NULL, OPTION_NO_WCHAR_SIZE_WARNING},
  { "fix-cortex-a53-835769", no_argument, NULL, OPTION_FIX_ERRATUM_835769},
  { "fix-cortex-a53-843419", optional_argument, NULL, OPTION_FIX_ERRATUM_843419},
  { "no-apply-dynamic-relocs", no_argument, NULL, OPTION_NO_APPLY_DYNAMIC_RELOCS},
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --no-enum-size-warning      Don'\''t warn about objects with incompatible\n"
		   "                                enum sizes\n"));
  fprintf (file, _("  --no-wchar-size-warning     Don'\''t warn about objects with incompatible\n"
		   "                                wchar_t sizes\n"));
  fprintf (file, _("  --pic-veneer                Always generate PIC interworking veneers\n"));
  fprintf (file, _("\
  --stub-group-size=N         Maximum size of a group of input sections that\n\
                                can be handled by one stub section.  A negative\n\
                                value locates all stubs after their branches\n\
                                (with a group size of -N), while a positive\n\
                                value allows two groups of input sections, one\n\
                                before, and one after each stub section.\n\
                                Values of +/-1 indicate the linker should\n\
                                choose suitable defaults.\n"));
  fprintf (file, _("  --fix-cortex-a53-835769      Fix erratum 835769\n"));
  fprintf (file, _("\
  --fix-cortex-a53-843419[=full|adr|adrp]      Fix erratum 843419 and optionally specify which workaround to use.\n\
                                               full (default): Use both ADRP and ADR workaround, this will \n\
                                                 increase the size of your binaries.\n\
                                               adr: Only use the ADR workaround, this will not cause any increase\n\
                                                 in binary size but linking will fail if the referenced address is\n\
                                                 out of range of an ADR instruction.  This will remove the need of using\n\
                                                 a veneer and results in both performance and size benefits.\n\
                                               adrp: Use only the ADRP workaround, this will never rewrite your ADRP\n\
                                                 instruction into an ADR.  As such the workaround will always use a\n\
                                                 veneer and this will give you both a performance and size overhead.\n"));
  fprintf (file, _("  --no-apply-dynamic-relocs    Do not apply link-time values for dynamic relocations\n"));
  fprintf (file, _("  -z force-bti                  Turn on Branch Target Identification mechanism and generate PLTs with BTI. Generate warnings for missing BTI on inputs\n"));
  fprintf (file, _("  -z pac-plt                    Protect PLTs with Pointer Authentication.\n"));
'

PARSE_AND_LIST_ARGS_CASE_Z_AARCH64='
      else if (strcmp (optarg, "force-bti") == 0)
	{
	  plt_type |= PLT_BTI;
	  bti_type = BTI_WARN;
	}
      else if (strcmp (optarg, "pac-plt") == 0)
	plt_type |= PLT_PAC;
'
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_AARCH64"

PARSE_AND_LIST_ARGS_CASES='
    case '\'p\'':
      /* Only here for backwards compatibility.  */
      break;

    case OPTION_NO_ENUM_SIZE_WARNING:
      no_enum_size_warning = 1;
      break;

    case OPTION_NO_WCHAR_SIZE_WARNING:
      no_wchar_size_warning = 1;
      break;

    case OPTION_PIC_VENEER:
      pic_veneer = 1;
      break;

    case OPTION_FIX_ERRATUM_835769:
      fix_erratum_835769 = 1;
      break;

    case OPTION_FIX_ERRATUM_843419:
      fix_erratum_843419 = ERRAT_ADR | ERRAT_ADRP;
      if (optarg && *optarg)
	{
	  if (strcmp ("full", optarg) == 0)
	    fix_erratum_843419 = ERRAT_ADR | ERRAT_ADRP;
	  else if (strcmp ("adrp", optarg) == 0)
	    fix_erratum_843419 = ERRAT_ADRP;
	  else if (strcmp ("adr", optarg) == 0)
	    fix_erratum_843419 = ERRAT_ADR;
	  else
	    einfo (_("%P: error: unrecognized option for "
		     "--fix-cortex-a53-843419: %s\n"), optarg);
	}
      break;

    case OPTION_NO_APPLY_DYNAMIC_RELOCS:
      no_apply_dynamic_relocs = 1;
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

# We have our own before_allocation etc. functions, but they call
# the standard routines, so give them a different name.
LDEMUL_BEFORE_ALLOCATION=aarch64_elf_before_allocation
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=aarch64_elf_create_output_section_statements

# Replace the elf before_parse function with our own.
LDEMUL_BEFORE_PARSE=gld"${EMULATION_NAME}"_before_parse

# Call the extra arm-elf function
LDEMUL_FINISH=gld${EMULATION_NAME}_finish
