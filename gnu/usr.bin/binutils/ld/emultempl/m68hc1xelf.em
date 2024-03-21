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

# This file is sourced from elf.em, and defines extra m68hc12-elf
# and m68hc11-elf specific routines.  It is used to generate the
# HC11/HC12 trampolines to call a far function by using a normal 'jsr/bsr'.
#
# - The HC11/HC12 relocations are checked to see if there is a
#   R_M68HC11_16 relocation to a symbol marked with STO_M68HC12_FAR.
#   This relocation cannot be made on the symbol but must be made on
#   its trampoline
#   The trampolines to generate are collected during this pass
#   (See elf32_m68hc11_size_stubs)
#
# - The trampolines are generated in a ".tramp" section.  The generation
#   takes care of HC11 and HC12 specificities.
#   (See elf32_m68hc11_build_stubs)
#
# - During relocation the R_M68HC11_16 relocation to the far symbols
#   are redirected to the trampoline that was generated.
#
# Copied from hppaelf and adapted for M68HC11/M68HC12 specific needs.
#
fragment <<EOF

#include "ldctor.h"
#include "elf32-m68hc1x.h"

static asection *m68hc11elf_add_stub_section (const char *, asection *);

/* Fake input file for stubs.  */
static lang_input_statement_type *stub_file;

/* By default the HC11/HC12 trampolines to call a far function using
   a normal 'bsr' and 'jsr' convention are generated during the link.
   The --no-trampoline option prevents that.  */
static int no_trampoline = 0;

/* Name of memory bank window in the MEMORY description.
   This is set by --bank-window option.  */
static const char* bank_window_name = 0;

static void
m68hc11_elf_${EMULATION_NAME}_before_allocation (void)
{
  lang_memory_region_type* region;
  int ret;

  gld${EMULATION_NAME}_before_allocation ();

  if (bfd_get_flavour (link_info.output_bfd) != bfd_target_elf_flavour)
    return;

  /* If generating a relocatable output file, then we don't
     have to generate the trampolines.  */
  if (bfd_link_relocatable (&link_info))
    return;

  ret = elf32_m68hc11_setup_section_lists (link_info.output_bfd, &link_info);
  if (ret != 0 && no_trampoline == 0)
    {
      if (ret < 0)
	{
	  einfo (_("%X%P: can not size stub section: %E\n"));
	  return;
	}

      /* Call into the BFD backend to do the real work.  */
      if (!elf32_m68hc11_size_stubs (link_info.output_bfd,
				     stub_file->the_bfd,
				     &link_info,
				     &m68hc11elf_add_stub_section))
	{
	  einfo (_("%X%P: can not size stub section: %E\n"));
	  return;
	}
    }

  if (bank_window_name == 0)
    return;

  /* The 'bank_window_name' memory region is a special region that describes
     the memory bank window to access to paged memory.  For 68HC12
     this is fixed and should be:

     window (rx) : ORIGIN = 0x8000, LENGTH = 16K

     But for 68HC11 this is board specific.  The definition of such
     memory region allows one to control how this paged memory is accessed.  */
  region = lang_memory_region_lookup (bank_window_name, false);

  /* Check the length to see if it was defined in the script.  */
  if (region->length != 0)
    {
      struct m68hc11_page_info *pinfo;
      unsigned i;

      /* Get default values  */
      m68hc11_elf_get_bank_parameters (&link_info);
      pinfo = &m68hc11_elf_hash_table (&link_info)->pinfo;

      /* And override them with the region definition.  */
      pinfo->bank_size = region->length;
      pinfo->bank_shift = 0;
      for (i = pinfo->bank_size; i != 0; i >>= 1)
	pinfo->bank_shift++;
      pinfo->bank_shift--;
      pinfo->bank_size = 1L << pinfo->bank_shift;
      pinfo->bank_mask = (1 << pinfo->bank_shift) - 1;
      pinfo->bank_physical = region->origin;
      pinfo->bank_physical_end = region->origin + pinfo->bank_size;

      if (pinfo->bank_size != region->length)
	{
	  einfo (_("%P: warning: the size of the 'window' memory region "
		   "is not a power of 2; its size %d is truncated to %d\n"),
		 region->length, pinfo->bank_size);
	}
    }
}

/* This is called before the input files are opened.  We create a new
   fake input file to hold the stub sections.  */

static void
m68hc11elf_create_output_section_statements (void)
{
  if (bfd_get_flavour (link_info.output_bfd) != bfd_target_elf_flavour)
    {
      einfo (_("%X%P: changing output format whilst linking "
	       "is not supported\n"));
      return;
    }

  stub_file = lang_add_input_file ("linker stubs",
				   lang_input_file_is_fake_enum,
				   NULL);
  stub_file->the_bfd = bfd_create ("linker stubs", link_info.output_bfd);
  if (stub_file->the_bfd == NULL
      || !bfd_set_arch_mach (stub_file->the_bfd,
			     bfd_get_arch (link_info.output_bfd),
			     bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%F%P: can not create BFD: %E\n"));
      return;
    }

  ldlang_add_file (stub_file);
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
	  if (l->input_section.section == info->input_section
	      || strcmp (bfd_section_name (l->input_section.section),
			 bfd_section_name (info->input_section)) == 0)
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


/* Call-back for elf32_m68hc11_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately before INPUT_SECTION.  */

static asection *
m68hc11elf_add_stub_section (const char *stub_sec_name,
			     asection *tramp_section)
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

  output_section = tramp_section->output_section;
  os = lang_output_section_get (output_section);

  /* Try to put the new section at the same place as an existing
     .tramp section.  Such .tramp section exists in most cases and
     contains the trampoline code.  This way we put the generated trampoline
     at the correct place.  */
  info.input_section = tramp_section;
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

/* For the 68HC12 we use this opportunity to build linker stubs.  */

static void
m68hc11elf_after_allocation (void)
{
  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour)
    {
      /* Now build the linker stubs.  */
      if (stub_file->the_bfd->sections != NULL)
	{
	  /* Call again the trampoline analyzer to initialize the trampoline
	     stubs with the correct symbol addresses.  Since there could have
	     been relaxation, the symbol addresses that were found during
	     first call may no longer be correct.  */
	  if (!elf32_m68hc11_size_stubs (link_info.output_bfd,
					 stub_file->the_bfd,
					 &link_info, 0))
	    {
	      einfo (_("%X%P: can not size stub section: %E\n"));
	      return;
	    }
	  if (!elf32_m68hc11_build_stubs (link_info.output_bfd, &link_info))
	    einfo (_("%X%P: can not build stubs: %E\n"));
	}
    }

  gld${EMULATION_NAME}_after_allocation ();
}

EOF

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_NO_TRAMPOLINE		300
#define OPTION_BANK_WINDOW		301
'

# The options are repeated below so that no abbreviations are allowed.
# Otherwise -s matches stub-group-size
PARSE_AND_LIST_LONGOPTS='
  { "no-trampoline", no_argument, NULL, OPTION_NO_TRAMPOLINE },
  { "bank-window",   required_argument, NULL, OPTION_BANK_WINDOW },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _(
"  --no-trampoline             Do not generate the far trampolines used to call\n"
"                                a far function using 'jsr' or 'bsr'\n"));
  fprintf (file, _(
"  --bank-window NAME          Specify the name of the memory region describing\n"
"                                the layout of the memory bank window\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_NO_TRAMPOLINE:
      no_trampoline = 1;
      break;
    case OPTION_BANK_WINDOW:
      bank_window_name = optarg;
      break;
'

# Put these extra m68hc11elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_BEFORE_ALLOCATION=m68hc11_elf_${EMULATION_NAME}_before_allocation
LDEMUL_AFTER_ALLOCATION=m68hc11elf_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=m68hc11elf_create_output_section_statements
