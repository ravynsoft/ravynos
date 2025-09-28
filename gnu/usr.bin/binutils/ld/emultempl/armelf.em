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

# This file is sourced from elf.em, and defines extra arm-elf
# specific routines.
#
test -z "$TARGET2_TYPE" && TARGET2_TYPE="rel"
fragment <<EOF

#include "ldctor.h"
#include "elf/arm.h"
#include "elf32-arm.h"

static struct elf32_arm_params params =
{
  NULL,				/* thumb_entry_symbol */
  0,				/* byteswap_code */
  0${TARGET1_IS_REL},		/* target1_is_rel */
  "${TARGET2_TYPE}",		/* target2_type */
  0,				/* fix_v4bx */
  0,				/* use_blx */
  BFD_ARM_VFP11_FIX_DEFAULT,	/* vfp11_denorm_fix */
  BFD_ARM_STM32L4XX_FIX_NONE,	/* stm32l4xx_fix */
  0,				/* no_enum_size_warning */
  0,				/* no_wchar_size_warning */
  0,				/* pic_veneer */
  -1,				/* fix_cortex_a8 */
  1,				/* fix_arm1176 */
  -1,				/* merge_exidx_entries */
  0,				/* cmse_implib */
  NULL				/* in_implib_bfd */
};
static char *in_implib_filename = NULL;

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
gld${EMULATION_NAME}_set_symbols (void)
{
  /* PR 19106: The section resizing code in gldarmelf_after_allocation
     is effectively the same as relaxation, so prevent early memory
     region checks which produce bogus error messages.
     Note - this test has nothing to do with symbols.  It is just here
     because this is the first emulation routine that is called after
     the command line has been parsed.  */
  if (!bfd_link_relocatable (&link_info))
    TARGET_ENABLE_RELAXATION;
}

static void
arm_elf_before_allocation (void)
{
  bfd_elf32_arm_set_byteswap_code (&link_info, params.byteswap_code);

  /* Choose type of VFP11 erratum fix, or warn if specified fix is unnecessary
     due to architecture version.  */
  bfd_elf32_arm_set_vfp11_fix (link_info.output_bfd, &link_info);

  /* Choose type of STM32L4XX erratum fix, or warn if specified fix is
     unnecessary due to architecture version.  */
  bfd_elf32_arm_set_stm32l4xx_fix (link_info.output_bfd, &link_info);

  /* Auto-select Cortex-A8 erratum fix if it wasn't explicitly specified.  */
  bfd_elf32_arm_set_cortex_a8_fix (link_info.output_bfd, &link_info);

  /* Ensure the output sections of veneers needing a dedicated one is not
     removed.  */
  bfd_elf32_arm_keep_private_stub_output_sections (&link_info);

  /* We should be able to set the size of the interworking stub section.  We
     can't do it until later if we have dynamic sections, though.  */
  if (elf_hash_table (&link_info)->dynobj == NULL)
    {
      /* Here we rummage through the found bfds to collect glue information.  */
      LANG_FOR_EACH_INPUT_STATEMENT (is)
	{
	  /* Initialise mapping tables for code/data.  */
	  bfd_elf32_arm_init_maps (is->the_bfd);

	  if (!bfd_elf32_arm_process_before_allocation (is->the_bfd,
							&link_info)
	      || !bfd_elf32_arm_vfp11_erratum_scan (is->the_bfd, &link_info)
	      || !bfd_elf32_arm_stm32l4xx_erratum_scan (is->the_bfd,
							&link_info))
	    /* xgettext:c-format */
	    einfo (_("%P: errors encountered processing file %s\n"),
		   is->filename);
	}

      /* We have seen it all.  Allocate it, and carry on.  */
      bfd_elf32_arm_allocate_interworking_sections (& link_info);
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


/* Call-back for elf32_arm_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately after INPUT_SECTION.  */

static asection *
elf32_arm_add_stub_section (const char * stub_sec_name,
			    asection *   output_section,
			    asection *   after_input_section,
			    unsigned int alignment_power)
{
  asection *stub_sec;
  flagword flags;
  lang_output_section_statement_type *os;
  struct hook_stub_info info;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
	   | SEC_HAS_CONTENTS | SEC_RELOC | SEC_IN_MEMORY | SEC_KEEP
	   | SEC_LINKER_CREATED);
  stub_sec = bfd_make_section_anyway_with_flags (stub_file->the_bfd,
						 stub_sec_name, flags);
  if (stub_sec == NULL)
    goto err_ret;

  bfd_set_section_alignment (stub_sec, alignment_power);

  os = lang_output_section_get (output_section);

  info.input_section = after_input_section;
  lang_list_init (&info.add);
  lang_add_section (&info.add, stub_sec, NULL, NULL, os);

  if (info.add.head == NULL)
    goto err_ret;

  if (after_input_section == NULL)
    {
      lang_statement_union_type **lp = &os->children.head;
      lang_statement_union_type *l, *lprev = NULL;

      for (; (l = *lp) != NULL; lp = &l->header.next, lprev = l);

      if (lprev)
	lprev->header.next = info.add.head;
      else
	os->children.head = info.add.head;

      return stub_sec;
    }
  else
    {
      if (hook_in_stub (&info, &os->children.head))
	return stub_sec;
    }

 err_ret:
  einfo (_("%X%P: can not make stub section: %E\n"));
  return NULL;
}

/* Another call-back for elf_arm_size_stubs.  */

static void
gldarm_layout_sections_again (void)
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
	elf32_arm_next_input_section (& link_info, i);
    }
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
  int ret;

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

  if (elf32_arm_fix_exidx_coverage (sec_list, sec_count, &link_info,
				    params.merge_exidx_entries))
    need_laying_out = 1;

  free (sec_list);

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
      ret = elf32_arm_setup_section_lists (link_info.output_bfd, &link_info);
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
	  if (! elf32_arm_size_stubs (link_info.output_bfd,
				      stub_file->the_bfd,
				      & link_info,
				      group_size,
				      & elf32_arm_add_stub_section,
				      & gldarm_layout_sections_again))
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
  struct bfd_link_hash_entry * h;

  {
    LANG_FOR_EACH_INPUT_STATEMENT (is)
      {
	/* Figure out where VFP11 erratum veneers (and the labels returning
	   from same) have been placed.  */
	bfd_elf32_arm_vfp11_fix_veneer_locations (is->the_bfd, &link_info);

	 /* Figure out where STM32L4XX erratum veneers (and the labels returning
	   from them) have been placed.  */
	bfd_elf32_arm_stm32l4xx_fix_veneer_locations (is->the_bfd, &link_info);
      }
  }

  if (!bfd_link_relocatable (&link_info))
    {
      /* Now build the linker stubs.  */
      if (stub_file->the_bfd->sections != NULL)
	{
	  if (! elf32_arm_build_stubs (& link_info))
	    einfo (_("%X%P: can not build stubs: %E\n"));
	}
    }

  finish_default ();

  if (params.thumb_entry_symbol)
    {
      h = bfd_link_hash_lookup (link_info.hash, params.thumb_entry_symbol,
				false, false, true);
    }
  else
    {
      struct elf_link_hash_entry * eh;

      if (!entry_symbol.name || !is_elf_hash_table (link_info.hash))
	return;

      h = bfd_link_hash_lookup (link_info.hash, entry_symbol.name,
				false, false, true);
      eh = (struct elf_link_hash_entry *)h;
      if (!h || ARM_GET_SYM_BRANCH_TYPE (eh->target_internal)
		!= ST_BRANCH_TO_THUMB)
	return;
    }


  if (h != (struct bfd_link_hash_entry *) NULL
      && (h->type == bfd_link_hash_defined
	  || h->type == bfd_link_hash_defweak)
      && h->u.def.section->output_section != NULL)
    {
      static char buffer[32];
      bfd_vma val;

      /* Special procesing is required for a Thumb entry symbol.  The
	 bottom bit of its address must be set.  */
      val = (h->u.def.value
	     + bfd_section_vma (h->u.def.section->output_section)
	     + h->u.def.section->output_offset);

      val |= 1;

      /* Now convert this value into a string and store it in entry_symbol
	 where the lang_finish() function will pick it up.  */
      sprintf (buffer, "0x%" PRIx64, (uint64_t) val);

      if (params.thumb_entry_symbol != NULL && entry_symbol.name != NULL
	  && entry_from_cmdline)
	einfo (_("%P: warning: '--thumb-entry %s' is overriding '-e %s'\n"),
	       params.thumb_entry_symbol, entry_symbol.name);
      entry_symbol.name = buffer;
    }
  else
    einfo (_("%P: warning: cannot find thumb start symbol %s\n"),
	   h->root.string);
}

/* This is a convenient point to tell BFD about target specific flags.
   After the output has been created, but before inputs are read.  */
static void
arm_elf_create_output_section_statements (void)
{
  if (strstr (bfd_get_target (link_info.output_bfd), "arm") == NULL)
    {
      /* The arm backend needs special fields in the output hash structure.
	 These will only be created if the output format is an arm format,
	 hence we do not support linking and changing output formats at the
	 same time.  Use a link followed by objcopy to change output formats.  */
      einfo (_("%F%P: error: cannot change output format "
	       "whilst linking %s binaries\n"), "ARM");
      return;
    }

  if (in_implib_filename)
    {
      params.in_implib_bfd = bfd_openr (in_implib_filename,
					bfd_get_target (link_info.output_bfd));

      if (params.in_implib_bfd == NULL)
	einfo (_("%F%P: %s: can't open: %E\n"), in_implib_filename);

      if (!bfd_check_format (params.in_implib_bfd, bfd_object))
	einfo (_("%F%P: %s: not a relocatable file: %E\n"), in_implib_filename);
    }

  bfd_elf32_arm_set_target_params (link_info.output_bfd, &link_info, &params);

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

  /* Also use the stub file for stubs placed in a single output section.  */
  bfd_elf32_arm_add_glue_sections_to_bfd (stub_file->the_bfd, &link_info);
  bfd_elf32_arm_get_bfd_for_interworking (stub_file->the_bfd, &link_info);
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_THUMB_ENTRY		301
#define OPTION_BE8			302
#define OPTION_TARGET1_REL		303
#define OPTION_TARGET1_ABS		304
#define OPTION_TARGET2			305
#define OPTION_FIX_V4BX			306
#define OPTION_USE_BLX			307
#define OPTION_VFP11_DENORM_FIX		308
#define OPTION_NO_ENUM_SIZE_WARNING	309
#define OPTION_PIC_VENEER		310
#define OPTION_FIX_V4BX_INTERWORKING	311
#define OPTION_STUBGROUP_SIZE		312
#define OPTION_NO_WCHAR_SIZE_WARNING	313
#define OPTION_FIX_CORTEX_A8		314
#define OPTION_NO_FIX_CORTEX_A8		315
#define OPTION_NO_MERGE_EXIDX_ENTRIES	316
#define OPTION_FIX_ARM1176		317
#define OPTION_NO_FIX_ARM1176		318
#define OPTION_LONG_PLT			319
#define OPTION_STM32L4XX_FIX		320
#define OPTION_CMSE_IMPLIB		321
#define OPTION_IN_IMPLIB		322
'

PARSE_AND_LIST_SHORTOPTS=p

PARSE_AND_LIST_LONGOPTS='
  { "no-pipeline-knowledge", no_argument, NULL, '\'p\''},
  { "thumb-entry", required_argument, NULL, OPTION_THUMB_ENTRY},
  { "be8", no_argument, NULL, OPTION_BE8},
  { "target1-rel", no_argument, NULL, OPTION_TARGET1_REL},
  { "target1-abs", no_argument, NULL, OPTION_TARGET1_ABS},
  { "target2", required_argument, NULL, OPTION_TARGET2},
  { "fix-v4bx", no_argument, NULL, OPTION_FIX_V4BX},
  { "fix-v4bx-interworking", no_argument, NULL, OPTION_FIX_V4BX_INTERWORKING},
  { "use-blx", no_argument, NULL, OPTION_USE_BLX},
  { "vfp11-denorm-fix", required_argument, NULL, OPTION_VFP11_DENORM_FIX},
  { "fix-stm32l4xx-629360", optional_argument, NULL, OPTION_STM32L4XX_FIX},
  { "no-enum-size-warning", no_argument, NULL, OPTION_NO_ENUM_SIZE_WARNING},
  { "pic-veneer", no_argument, NULL, OPTION_PIC_VENEER},
  { "stub-group-size", required_argument, NULL, OPTION_STUBGROUP_SIZE },
  { "no-wchar-size-warning", no_argument, NULL, OPTION_NO_WCHAR_SIZE_WARNING},
  { "fix-cortex-a8", no_argument, NULL, OPTION_FIX_CORTEX_A8 },
  { "no-fix-cortex-a8", no_argument, NULL, OPTION_NO_FIX_CORTEX_A8 },
  { "no-merge-exidx-entries", no_argument, NULL, OPTION_NO_MERGE_EXIDX_ENTRIES },
  { "fix-arm1176", no_argument, NULL, OPTION_FIX_ARM1176 },
  { "no-fix-arm1176", no_argument, NULL, OPTION_NO_FIX_ARM1176 },
  { "long-plt", no_argument, NULL, OPTION_LONG_PLT },
  { "cmse-implib", no_argument, NULL, OPTION_CMSE_IMPLIB },
  { "in-implib", required_argument, NULL, OPTION_IN_IMPLIB },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("  --thumb-entry=<sym>         Set the entry point to be Thumb symbol <sym>\n"));
  fprintf (file, _("  --be8                       Output BE8 format image\n"));
  fprintf (file, _("  --target1-rel               Interpret R_ARM_TARGET1 as R_ARM_REL32\n"));
  fprintf (file, _("  --target1-abs               Interpret R_ARM_TARGET1 as R_ARM_ABS32\n"));
  fprintf (file, _("  --target2=<type>            Specify definition of R_ARM_TARGET2\n"));
  fprintf (file, _("  --fix-v4bx                  Rewrite BX rn as MOV pc, rn for ARMv4\n"));
  fprintf (file, _("  --fix-v4bx-interworking     Rewrite BX rn branch to ARMv4 interworking veneer\n"));
  fprintf (file, _("  --use-blx                   Enable use of BLX instructions\n"));
  fprintf (file, _("  --vfp11-denorm-fix          Specify how to fix VFP11 denorm erratum\n"));
  fprintf (file, _("  --fix-stm32l4xx-629360      Specify how to fix STM32L4XX 629360 erratum\n"));
  fprintf (file, _("  --no-enum-size-warning      Don'\''t warn about objects with incompatible\n"
		   "                                enum sizes\n"));
  fprintf (file, _("  --no-wchar-size-warning     Don'\''t warn about objects with incompatible\n"
		   "                                wchar_t sizes\n"));
  fprintf (file, _("  --pic-veneer                Always generate PIC interworking veneers\n"));
  fprintf (file, _("  --long-plt                  Generate long .plt entries\n"
           "                              to handle large .plt/.got displacements\n"));
  fprintf (file, _("  --cmse-implib               Make import library to be a secure gateway import\n"
                   "                                library as per ARMv8-M Security Extensions\n"));
  fprintf (file, _("  --in-implib                 Import library whose symbols address must\n"
                   "                                remain stable\n"));
  fprintf (file, _("\
  --stub-group-size=N         Maximum size of a group of input sections that\n\
                                can be handled by one stub section.  A negative\n\
                                value locates all stubs after their branches\n\
                                (with a group size of -N), while a positive\n\
                                value allows two groups of input sections, one\n\
                                before, and one after each stub section.\n\
                                Values of +/-1 indicate the linker should\n\
                                choose suitable defaults.\n"));
  fprintf (file, _("  --[no-]fix-cortex-a8        Disable/enable Cortex-A8 Thumb-2 branch erratum fix\n"));
  fprintf (file, _("  --no-merge-exidx-entries    Disable merging exidx entries\n"));
  fprintf (file, _("  --[no-]fix-arm1176          Disable/enable ARM1176 BLX immediate erratum fix\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case '\'p\'':
      /* Only here for backwards compatibility.  */
      break;

    case OPTION_THUMB_ENTRY:
      params.thumb_entry_symbol = optarg;
      break;

    case OPTION_BE8:
      params.byteswap_code = 1;
      break;

    case OPTION_TARGET1_REL:
      params.target1_is_rel = 1;
      break;

    case OPTION_TARGET1_ABS:
      params.target1_is_rel = 0;
      break;

    case OPTION_TARGET2:
      params.target2_type = optarg;
      break;

    case OPTION_FIX_V4BX:
      params.fix_v4bx = 1;
      break;

    case OPTION_FIX_V4BX_INTERWORKING:
      params.fix_v4bx = 2;
      break;

    case OPTION_USE_BLX:
      params.use_blx = 1;
      break;

    case OPTION_VFP11_DENORM_FIX:
      if (strcmp (optarg, "none") == 0)
	params.vfp11_denorm_fix = BFD_ARM_VFP11_FIX_NONE;
      else if (strcmp (optarg, "scalar") == 0)
	params.vfp11_denorm_fix = BFD_ARM_VFP11_FIX_SCALAR;
      else if (strcmp (optarg, "vector") == 0)
	params.vfp11_denorm_fix = BFD_ARM_VFP11_FIX_VECTOR;
      else
	einfo (_("%P: unrecognized VFP11 fix type '\''%s'\''\n"), optarg);
      break;

    case OPTION_STM32L4XX_FIX:
      if (!optarg)
	params.stm32l4xx_fix = BFD_ARM_STM32L4XX_FIX_DEFAULT;
      else if (strcmp (optarg, "none") == 0)
	params.stm32l4xx_fix = BFD_ARM_STM32L4XX_FIX_NONE;
      else if (strcmp (optarg, "default") == 0)
	params.stm32l4xx_fix = BFD_ARM_STM32L4XX_FIX_DEFAULT;
      else if (strcmp (optarg, "all") == 0)
	params.stm32l4xx_fix = BFD_ARM_STM32L4XX_FIX_ALL;
      else
	einfo (_("%P: unrecognized STM32L4XX fix type '\''%s'\''\n"), optarg);
      break;

    case OPTION_NO_ENUM_SIZE_WARNING:
      params.no_enum_size_warning = 1;
      break;

    case OPTION_NO_WCHAR_SIZE_WARNING:
      params.no_wchar_size_warning = 1;
      break;

    case OPTION_PIC_VENEER:
      params.pic_veneer = 1;
      break;

    case OPTION_STUBGROUP_SIZE:
      {
	const char *end;

	group_size = bfd_scan_vma (optarg, &end, 0);
	if (*end)
	  einfo (_("%F%P: invalid number `%s'\''\n"), optarg);
      }
      break;

    case OPTION_FIX_CORTEX_A8:
      params.fix_cortex_a8 = 1;
      break;

    case OPTION_NO_FIX_CORTEX_A8:
      params.fix_cortex_a8 = 0;
      break;

   case OPTION_NO_MERGE_EXIDX_ENTRIES:
      params.merge_exidx_entries = 0;
      break;

   case OPTION_FIX_ARM1176:
      params.fix_arm1176 = 1;
      break;

   case OPTION_NO_FIX_ARM1176:
      params.fix_arm1176 = 0;
      break;

   case OPTION_LONG_PLT:
      bfd_elf32_arm_use_long_plt ();
      break;

   case OPTION_CMSE_IMPLIB:
      params.cmse_implib = 1;
      break;

   case OPTION_IN_IMPLIB:
      in_implib_filename = optarg;
      break;
'

# We have our own before_allocation etc. functions, but they call
# the standard routines, so give them a different name.
LDEMUL_BEFORE_ALLOCATION=arm_elf_before_allocation
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=arm_elf_create_output_section_statements

# Replace the elf before_parse function with our own.
LDEMUL_BEFORE_PARSE=gld"${EMULATION_NAME}"_before_parse
LDEMUL_SET_SYMBOLS=gld"${EMULATION_NAME}"_set_symbols

# Call the extra arm-elf function
LDEMUL_FINISH=gld${EMULATION_NAME}_finish
