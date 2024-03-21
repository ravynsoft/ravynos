/* ldemul.c -- clearing house for ld emulation states
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"
#include "getopt.h"
#include "bfdlink.h"
#include "ctf-api.h"

#include "ld.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldemul.h"
#include "ldmain.h"
#include "ldemul-list.h"

static ld_emulation_xfer_type *ld_emulation;

void
ldemul_hll (char *name)
{
  ld_emulation->hll (name);
}

void
ldemul_syslib (char *name)
{
  ld_emulation->syslib (name);
}

void
ldemul_after_parse (void)
{
  ld_emulation->after_parse ();
}

void
ldemul_before_parse (void)
{
  ld_emulation->before_parse ();
}

void
ldemul_before_plugin_all_symbols_read (void)
{
  if (ld_emulation->before_plugin_all_symbols_read)
    ld_emulation->before_plugin_all_symbols_read ();
}

void
ldemul_after_open (void)
{
  ld_emulation->after_open ();
}

void
ldemul_after_check_relocs (void)
{
  ld_emulation->after_check_relocs ();
}

void
ldemul_before_place_orphans (void)
{
  ld_emulation->before_place_orphans ();
}

void
ldemul_after_allocation (void)
{
  ld_emulation->after_allocation ();
}

void
ldemul_before_allocation (void)
{
  ld_emulation->before_allocation ();
}

void
ldemul_set_output_arch (void)
{
  ld_emulation->set_output_arch ();
}

void
ldemul_finish (void)
{
  ld_emulation->finish ();
}

void
ldemul_set_symbols (void)
{
  if (ld_emulation->set_symbols)
    ld_emulation->set_symbols ();
}

void
ldemul_create_output_section_statements (void)
{
  if (ld_emulation->create_output_section_statements)
    ld_emulation->create_output_section_statements ();
}

char *
ldemul_get_script (int *isfile)
{
  return ld_emulation->get_script (isfile);
}

bool
ldemul_open_dynamic_archive (const char *arch, search_dirs_type *search,
			     lang_input_statement_type *entry)
{
  if (ld_emulation->open_dynamic_archive)
    return (*ld_emulation->open_dynamic_archive) (arch, search, entry);
  return false;
}

lang_output_section_statement_type *
ldemul_place_orphan (asection *s, const char *name, int constraint)
{
  if (ld_emulation->place_orphan)
    return (*ld_emulation->place_orphan) (s, name, constraint);
  return NULL;
}

void
ldemul_add_options (int ns, char **shortopts, int nl,
		    struct option **longopts, int nrl,
		    struct option **really_longopts)
{
  if (ld_emulation->add_options)
    (*ld_emulation->add_options) (ns, shortopts, nl, longopts,
				  nrl, really_longopts);
}

bool
ldemul_handle_option (int optc)
{
  if (ld_emulation->handle_option)
    return (*ld_emulation->handle_option) (optc);
  return false;
}

bool
ldemul_parse_args (int argc, char **argv)
{
  /* Try and use the emulation parser if there is one.  */
  if (ld_emulation->parse_args)
    return (*ld_emulation->parse_args) (argc, argv);
  return false;
}

/* Let the emulation code handle an unrecognized file.  */

bool
ldemul_unrecognized_file (lang_input_statement_type *entry)
{
  if (ld_emulation->unrecognized_file)
    return (*ld_emulation->unrecognized_file) (entry);
  return false;
}

/* Let the emulation code handle a recognized file.  */

bool
ldemul_recognized_file (lang_input_statement_type *entry)
{
  if (ld_emulation->recognized_file)
    return (*ld_emulation->recognized_file) (entry);
  return false;
}

char *
ldemul_choose_target (int argc, char **argv)
{
  return ld_emulation->choose_target (argc, argv);
}


/* The default choose_target function.  */

char *
ldemul_default_target (int argc ATTRIBUTE_UNUSED, char **argv ATTRIBUTE_UNUSED)
{
  char *from_outside = getenv (TARGET_ENVIRON);
  if (from_outside != (char *) NULL)
    return from_outside;
  return ld_emulation->target_name;
}

/* If the entry point was not specified as an address, then add the
   symbol as undefined.  This will cause ld to extract an archive
   element defining the entry if ld is linking against such an archive.

   We don't do this when generating shared libraries unless given -e
   on the command line, because most shared libs are not designed to
   be run as an executable.  However, some are, eg. glibc ld.so and
   may rely on the default linker script supplying ENTRY.  So we can't
   remove the ENTRY from the script, but would rather not insert
   undefined _start syms.  */

void
after_parse_default (void)
{
  if (entry_symbol.name != NULL
      && (bfd_link_executable (&link_info) || entry_from_cmdline))
    {
      bool is_vma = false;

      if (entry_from_cmdline)
	{
	  const char *send;

	  bfd_scan_vma (entry_symbol.name, &send, 0);
	  is_vma = *send == '\0';
	}
      if (!is_vma)
	ldlang_add_undef (entry_symbol.name, entry_from_cmdline);
    }
  if (link_info.maxpagesize == 0)
    link_info.maxpagesize = bfd_emul_get_maxpagesize (default_target);
  if (link_info.commonpagesize == 0)
    link_info.commonpagesize = bfd_emul_get_commonpagesize (default_target);
}

void
after_open_default (void)
{
  link_info.big_endian = true;

  if (bfd_big_endian (link_info.output_bfd))
    ;
  else if (bfd_little_endian (link_info.output_bfd))
    link_info.big_endian = false;
  else
    {
      if (command_line.endian == ENDIAN_BIG)
	;
      else if (command_line.endian == ENDIAN_LITTLE)
	link_info.big_endian = false;
      else if (command_line.endian == ENDIAN_UNSET)
	{
	  LANG_FOR_EACH_INPUT_STATEMENT (s)
	    if (s->the_bfd != NULL)
	      {
		if (bfd_little_endian (s->the_bfd))
		  link_info.big_endian = false;
		break;
	      }
	}
    }
}

void
after_check_relocs_default (void)
{
}

void
before_place_orphans_default (void)
{
}

void
after_allocation_default (void)
{
  lang_relax_sections (false);
}

void
before_allocation_default (void)
{
  if (!bfd_link_relocatable (&link_info))
    strip_excluded_output_sections ();
}

void
finish_default (void)
{
  if (!bfd_link_relocatable (&link_info))
    _bfd_fix_excluded_sec_syms (link_info.output_bfd, &link_info);
}

void
set_output_arch_default (void)
{
  /* Set the output architecture and machine if possible.  */
  bfd_set_arch_mach (link_info.output_bfd,
		     ldfile_output_architecture, ldfile_output_machine);
}

void
syslib_default (char *ignore ATTRIBUTE_UNUSED)
{
  info_msg (_("%pS SYSLIB ignored\n"), NULL);
}

void
hll_default (char *ignore ATTRIBUTE_UNUSED)
{
  info_msg (_("%pS HLL ignored\n"), NULL);
}

ld_emulation_xfer_type *ld_emulations[] = { EMULATION_LIST };

void
ldemul_choose_mode (char *target)
{
  ld_emulation_xfer_type **eptr = ld_emulations;
  /* Ignore "gld" prefix.  */
  if (target[0] == 'g' && target[1] == 'l' && target[2] == 'd')
    target += 3;
  for (; *eptr; eptr++)
    {
      if (strcmp (target, (*eptr)->emulation_name) == 0)
	{
	  ld_emulation = *eptr;
	  return;
	}
    }
  einfo (_("%P: unrecognised emulation mode: %s\n"), target);
  einfo (_("Supported emulations: "));
  ldemul_list_emulations (stderr);
  einfo ("%F\n");
}

void
ldemul_list_emulations (FILE *f)
{
  ld_emulation_xfer_type **eptr = ld_emulations;
  bool first = true;

  for (; *eptr; eptr++)
    {
      if (first)
	first = false;
      else
	fprintf (f, " ");
      fprintf (f, "%s", (*eptr)->emulation_name);
    }
}

void
ldemul_list_emulation_options (FILE *f)
{
  ld_emulation_xfer_type **eptr;
  int options_found = 0;

  for (eptr = ld_emulations; *eptr; eptr++)
    {
      ld_emulation_xfer_type *emul = *eptr;

      if (emul->list_options)
	{
	  fprintf (f, "%s: \n", emul->emulation_name);

	  emul->list_options (f);

	  options_found = 1;
	}
    }

  if (!options_found)
    fprintf (f, _("  no emulation specific options.\n"));
}

int
ldemul_find_potential_libraries (char *name, lang_input_statement_type *entry)
{
  if (ld_emulation->find_potential_libraries)
    return ld_emulation->find_potential_libraries (name, entry);

  return 0;
}

struct bfd_elf_version_expr *
ldemul_new_vers_pattern (struct bfd_elf_version_expr *entry)
{
  if (ld_emulation->new_vers_pattern)
    entry = (*ld_emulation->new_vers_pattern) (entry);
  return entry;
}

void
ldemul_extra_map_file_text (bfd *abfd, struct bfd_link_info *info, FILE *mapf)
{
  if (ld_emulation->extra_map_file_text)
    ld_emulation->extra_map_file_text (abfd, info, mapf);
}

int
ldemul_emit_ctf_early (void)
{
  if (ld_emulation->emit_ctf_early)
    return ld_emulation->emit_ctf_early ();
  /* If the emulation doesn't know if it wants to emit CTF early, it is going
     to do so.  */
  return 1;
}

void
ldemul_acquire_strings_for_ctf (struct ctf_dict *ctf_output,
				struct elf_strtab_hash *symstrtab)
{
  if (ld_emulation->acquire_strings_for_ctf)
    ld_emulation->acquire_strings_for_ctf (ctf_output, symstrtab);
}

void
ldemul_new_dynsym_for_ctf (struct ctf_dict *ctf_output, int symidx,
			   struct elf_internal_sym *sym)
{
  if (ld_emulation->new_dynsym_for_ctf)
    ld_emulation->new_dynsym_for_ctf (ctf_output, symidx, sym);
}

bool
ldemul_print_symbol (struct bfd_link_hash_entry *hash_entry, void *ptr)
{
  if (ld_emulation->print_symbol)
    return ld_emulation->print_symbol (hash_entry, ptr);
  return print_one_symbol (hash_entry, ptr);
}
