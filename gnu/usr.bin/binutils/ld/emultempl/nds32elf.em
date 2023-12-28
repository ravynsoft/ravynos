# This shell script emits a C file. -*- C -*-
# Copyright (C) 2012-2023 Free Software Foundation, Inc.
# Contributed by Andes Technology Corporation.
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

fragment <<EOF

#include "elf-bfd.h"
#include "elf/nds32.h"
#include <stdint.h>
#include "elf32-nds32.h"

static int relax_fp_as_gp = 1;		/* --mrelax-omit-fp  */
static int eliminate_gc_relocs = 0;	/* --meliminate-gc-relocs  */
static FILE *sym_ld_script = NULL;	/* --mgen-symbol-ld-script=<file>  */
static int hyper_relax = 1;		/* --mhyper-relax  */
static int tls_desc_trampoline = 0;	/* --m[no]tlsdesc-trampoline.  */
/* Disable if linking a dynamically linked executable.  */
static int load_store_relax = 1;

/* Save the target options into output bfd to avoid using to many global
   variables. Do this after the output has been created, but before
   inputs are read.  */
static void
nds32_elf_create_output_section_statements (void)
{
  if (strstr (bfd_get_target (link_info.output_bfd), "nds32") == NULL)
    {
      /* Check the output target is nds32.  */
      einfo (_("%F%P: error: cannot change output format whilst "
	       "linking %s binaries\n"), "NDS32");
      return;
    }

  bfd_elf32_nds32_set_target_option (&link_info,
				     relax_fp_as_gp,
				     eliminate_gc_relocs,
				     sym_ld_script,
				     hyper_relax,
				     tls_desc_trampoline,
				     load_store_relax);
}

static void
nds32_elf_after_parse (void)
{
  if (bfd_link_relocatable (&link_info)
      || bfd_link_pic (&link_info))
    DISABLE_RELAXATION;

  if (!RELAXATION_ENABLED)
    relax_fp_as_gp = 0;

  ldelf_after_parse ();
}

static void
nds32_elf_after_open (void)
{
  unsigned int arch_ver = (unsigned int)-1;
  unsigned int abi_ver = (unsigned int)-1;
  bfd *abfd;

  /* For now, make sure all object files are of the same architecture.
     We may try to merge object files with different architecture together.  */
  for (abfd = link_info.input_bfds; abfd != NULL; abfd = abfd->link.next)
    {
      if (arch_ver == (unsigned int)-1 && E_N1_ARCH != (elf_elfheader (abfd)->e_flags & EF_NDS_ARCH))
	arch_ver = elf_elfheader (abfd)->e_flags & EF_NDS_ARCH ;

      if (abi_ver == (unsigned int)-1)
	{
	  /* Initialize ABI version, if not ABI0.
	     (OS uses empty file to create empty ELF with ABI0).  */
	  if ((elf_elfheader (abfd)->e_flags & EF_NDS_ABI) != 0)
	    abi_ver = elf_elfheader (abfd)->e_flags & EF_NDS_ABI ;
	}
      else if ((elf_elfheader (abfd)->e_flags & EF_NDS_ABI) != 0
	       && abi_ver != (elf_elfheader (abfd)->e_flags & EF_NDS_ABI))
	{
	  /* Incompatible objects.  */
	  einfo (_("%F%P: %pB: ABI version of object files mismatched\n"),
		 abfd);
	}
    }

  /* Check object files if the target is dynamic linked executable
     or shared object.  */
  if (elf_hash_table (&link_info)->dynamic_sections_created
      || bfd_link_pic (&link_info)
      || bfd_link_pie (&link_info))
    {
      /* Dynamic linked executable with SDA and non-PIC.
	 Turn off load/store relaxtion.  */
      /* This may support in the future.  */
      load_store_relax = 0 ;
      relax_fp_as_gp = 0;
    }

  /* Call the standard elf routine.  */
  gld${EMULATION_NAME}_after_open ();
}

static void
nds32_elf_after_allocation (void)
{
  /* Call default after allocation callback.
     1. This is where relaxation is done.
     2. It calls ldelf_map_segments to build ELF segment table.
     3. Any relaxation requires relax being done must be called after it.  */
  gld${EMULATION_NAME}_after_allocation ();
}

EOF
# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_BASELINE			301
#define OPTION_ELIM_GC_RELOCS		(OPTION_BASELINE + 1)
#define OPTION_FP_AS_GP			(OPTION_BASELINE + 2)
#define OPTION_NO_FP_AS_GP		(OPTION_BASELINE + 3)
#define OPTION_REDUCE_FP_UPDATE		(OPTION_BASELINE + 4)
#define OPTION_NO_REDUCE_FP_UPDATE	(OPTION_BASELINE + 5)
#define OPTION_EXPORT_SYMBOLS		(OPTION_BASELINE + 6)
#define OPTION_HYPER_RELAX		(OPTION_BASELINE + 7)
#define OPTION_TLSDESC_TRAMPOLINE	(OPTION_BASELINE + 8)
#define OPTION_NO_TLSDESC_TRAMPOLINE	(OPTION_BASELINE + 9)
'
PARSE_AND_LIST_LONGOPTS='
  { "mfp-as-gp", no_argument, NULL, OPTION_FP_AS_GP},
  { "mno-fp-as-gp", no_argument, NULL, OPTION_NO_FP_AS_GP},
  { "mexport-symbols", required_argument, NULL, OPTION_EXPORT_SYMBOLS},
  { "mhyper-relax", required_argument, NULL, OPTION_HYPER_RELAX},
  { "mtlsdesc-trampoline", no_argument, NULL, OPTION_TLSDESC_TRAMPOLINE},
  { "mno-tlsdesc-trampoline", no_argument, NULL, OPTION_NO_TLSDESC_TRAMPOLINE},
  /* These are deprecated options.  Remove them in the future.  */
  { "mrelax-reduce-fp-update", no_argument, NULL, OPTION_REDUCE_FP_UPDATE},
  { "mrelax-no-reduce-fp-update", no_argument, NULL, OPTION_NO_REDUCE_FP_UPDATE},
  { "mbaseline", required_argument, NULL, OPTION_BASELINE},
  { "meliminate-gc-relocs", no_argument, NULL, OPTION_ELIM_GC_RELOCS},
  { "mrelax-omit-fp", no_argument, NULL, OPTION_FP_AS_GP},
  { "mrelax-no-omit-fp", no_argument, NULL, OPTION_NO_FP_AS_GP},
  { "mgen-symbol-ld-script", required_argument, NULL, OPTION_EXPORT_SYMBOLS},
'
PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("\
  --m[no-]fp-as-gp            Disable/enable fp-as-gp relaxation\n"));
  fprintf (file, _("\
  --mexport-symbols=FILE      Exporting symbols in linker script\n"));
  fprintf (file, _("\
  --mhyper-relax=level        Adjust relax level (low|medium|high). default: medium\n"));
  fprintf (file, _("\
  --m[no-]tlsdesc-trampoline  Disable/enable TLS DESC trampoline\n"));
'
PARSE_AND_LIST_ARGS_CASES='
  case OPTION_BASELINE:
    einfo (_("%P: --mbaseline is not used anymore\n"));
    break;
  case OPTION_ELIM_GC_RELOCS:
    eliminate_gc_relocs = 1;
    break;
  case OPTION_FP_AS_GP:
  case OPTION_NO_FP_AS_GP:
    relax_fp_as_gp = (optc == OPTION_FP_AS_GP);
    break;
  case OPTION_REDUCE_FP_UPDATE:
  case OPTION_NO_REDUCE_FP_UPDATE:
    einfo (_("%P: --relax-[no-]reduce-fp-updat is not used anymore\n"));
    break;
  case OPTION_EXPORT_SYMBOLS:
    if (!optarg)
      einfo (_("%P: missing file for --mexport-symbols\n"), optarg);

    if(strcmp (optarg, "-") == 0)
      sym_ld_script = stdout;
    else
      {
	sym_ld_script = fopen (optarg, FOPEN_WT);
	if(sym_ld_script == NULL)
	  einfo (_("%F%P: cannot open map file %s: %E\n"), optarg);
      }
    break;
  case OPTION_HYPER_RELAX:
    if (!optarg)
      einfo (_("%P: valid arguments to --mhyper-relax=(low|medium|high)\n"));

    if (strcmp (optarg, "low") == 0)
      hyper_relax = 0;
    else if (strcmp (optarg, "medium") == 0)
      hyper_relax = 1;
    else if (strcmp (optarg, "high") == 0)
      hyper_relax = 2;
    else
      einfo (_("%P: valid arguments to --mhyper-relax=(low|medium|high)\n"));

    break;
  case OPTION_TLSDESC_TRAMPOLINE:
    tls_desc_trampoline = 1;
    break;
  case OPTION_NO_TLSDESC_TRAMPOLINE:
    tls_desc_trampoline = 0;
     break;
'
LDEMUL_AFTER_OPEN=nds32_elf_after_open
LDEMUL_AFTER_PARSE=nds32_elf_after_parse
LDEMUL_AFTER_ALLOCATION=nds32_elf_after_allocation
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=nds32_elf_create_output_section_statements
