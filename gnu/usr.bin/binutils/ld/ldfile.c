/* Linker file opening and searching.
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
#include "bfdlink.h"
#include "ctf-api.h"
#include "safe-ctype.h"
#include "ld.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldfile.h"
#include "ldmain.h"
#include <ldgram.h>
#include "ldlex.h"
#include "ldemul.h"
#include "libiberty.h"
#include "filenames.h"
#include <fnmatch.h>
#if BFD_SUPPORTS_PLUGINS
#include "plugin-api.h"
#include "plugin.h"
#endif /* BFD_SUPPORTS_PLUGINS */

bool ldfile_assumed_script = false;
const char *ldfile_output_machine_name = "";
unsigned long ldfile_output_machine;
enum bfd_architecture ldfile_output_architecture;
search_dirs_type *search_head;

#ifdef VMS
static char *slash = "";
#else
#if defined (_WIN32) && !defined (__CYGWIN32__)
static char *slash = "\\";
#else
static char *slash = "/";
#endif
#endif

typedef struct search_arch
{
  char *name;
  struct search_arch *next;
} search_arch_type;

static search_dirs_type **search_tail_ptr = &search_head;
static search_arch_type *search_arch_head;
static search_arch_type **search_arch_tail_ptr = &search_arch_head;

typedef struct input_remap
{
  const char *          pattern;  /* Pattern to match input files.  */
  const char *          renamed;  /* Filename to use if the pattern matches.  */
  struct input_remap *  next;     /* Link in a chain of these structures.  */
} input_remap;

static struct input_remap * input_remaps = NULL;

void
ldfile_add_remap (const char * pattern, const char * renamed)
{
  struct input_remap * new_entry;

  new_entry = xmalloc (sizeof * new_entry);
  new_entry->pattern = xstrdup (pattern);
  new_entry->next = NULL;

  /* Look for special filenames that mean that the input file should be ignored.  */
  if (strcmp (renamed, "/dev/null") == 0
      || strcmp (renamed, "NUL") == 0)
    new_entry->renamed = NULL;
  else
    /* FIXME: Should we add sanity checking of the 'renamed' string ?  */
    new_entry->renamed = xstrdup (renamed);

  /* It would be easier to add this new node at the start of the chain,
     but users expect that remapping will occur in the order in which
     they occur on the command line, and in the remapping files.  */
  if (input_remaps == NULL)
    {
      input_remaps = new_entry;
    }
  else
    {
      struct input_remap * i;

      for (i = input_remaps; i->next != NULL; i = i->next)
	;
      i->next = new_entry;
    }
}

void
ldfile_remap_input_free (void)
{
  while (input_remaps != NULL)
    {
      struct input_remap * i = input_remaps;

      input_remaps = i->next;
      free ((void *) i->pattern);
      free ((void *) i->renamed);
      free (i);
    }
}

bool
ldfile_add_remap_file (const char * file)
{
  FILE * f;

  f = fopen (file, FOPEN_RT);
  if (f == NULL)
    return false;

  size_t linelen = 256;
  char * line = xmalloc (linelen);

  do
    {
      char * p = line;
      char * q;

      /* Normally this would use getline(3), but we need to be portable.  */
      while ((q = fgets (p, linelen - (p - line), f)) != NULL
	     && strlen (q) == linelen - (p - line) - 1
	     && line[linelen - 2] != '\n')
	{
	  line = xrealloc (line, 2 * linelen);
	  p = line + linelen - 1;
	  linelen += linelen;
	}

      if (q == NULL && p == line)
	break;

      p = strchr (line, '\n');
      if (p)
	*p = '\0';

      /* Because the file format does not know any form of quoting we
	 can search forward for the next '#' character and if found
	 make it terminating the line.  */
      p = strchr (line, '#');
      if (p)
	*p = '\0';

      /* Remove leading whitespace.  NUL is no whitespace character.  */
      p = line;
      while (*p == ' ' || *p == '\f' || *p == '\r' || *p == '\t' || *p == '\v')
	++p;

      /* If the line is blank it is ignored.  */
      if (*p == '\0')
	continue;

      char * pattern = p;

      /* Advance past the pattern.  We accept whitespace or '=' as an
	 end-of-pattern marker.  */
      while (*p && *p != '=' && *p != ' ' && *p != '\t' && *p != '\f'
	     && *p != '\r' && *p != '\v')
	++p;

      if (*p == '\0')
	{
	  einfo ("%F%P: malformed remap file entry: %s\n", line);
	  continue;
	}

      * p++ = '\0';

      /* Skip whitespace again.  */
      while (*p == ' ' || *p == '\f' || *p == '\r' || *p == '\t' || *p == '\v')
	++p;

      if (*p == '\0')
	{
	  einfo ("%F%P: malformed remap file entry: %s\n", line);
	  continue;
	}

      char * renamed = p;

      /* Advance past the rename entry.  */
      while (*p && *p != '=' && *p != ' ' && *p != '\t' && *p != '\f'
	     && *p != '\r' && *p != '\v')
	++p;
      /* And terminate it.  */
      *p = '\0';

      ldfile_add_remap (pattern, renamed);
    }
  while (! feof (f));

  free (line);
  fclose (f);

  return true;
}

const char *
ldfile_possibly_remap_input (const char * filename)
{
  struct input_remap * i;

  if (filename == NULL)
    return NULL;

  for (i = input_remaps; i != NULL; i = i->next)
    {
      if (fnmatch (i->pattern, filename, 0) == 0)
	{
	  if (verbose)
	    {
	      if (strpbrk ((i->pattern), "?*[") != NULL)
		{
		  if (i->renamed)
		    info_msg (_("remap input file '%s' to '%s' based upon pattern '%s'\n"),
			      filename, i->renamed, i->pattern);
		  else
		    info_msg (_("remove input file '%s' based upon pattern '%s'\n"),
			      filename, i->pattern);
		}
	      else
		{
		  if (i->renamed)
		    info_msg (_("remap input file '%s' to '%s'\n"),
			      filename, i->renamed);
		  else
		    info_msg (_("remove input file '%s'\n"),
			      filename);
		}
	    }

	  return i->renamed;
	}
    }
	 
  return filename;
}

void
ldfile_print_input_remaps (void)
{
  if (input_remaps == NULL)
    return;

  minfo (_("\nInput File Remapping\n\n"));

  struct input_remap * i;

  for (i = input_remaps; i != NULL; i = i->next)
    minfo (_("  Pattern: %s\tMaps To: %s\n"), i->pattern,
	   i->renamed ? i->renamed : _("<discard>"));
}


/* Test whether a pathname, after canonicalization, is the same or a
   sub-directory of the sysroot directory.  */

static bool
is_sysrooted_pathname (const char *name)
{
  char *realname;
  int len;
  bool result;

  if (ld_canon_sysroot == NULL)
    return false;

  realname = lrealpath (name);
  len = strlen (realname);
  result = false;
  if (len > ld_canon_sysroot_len
      && IS_DIR_SEPARATOR (realname[ld_canon_sysroot_len]))
    {
      realname[ld_canon_sysroot_len] = '\0';
      result = FILENAME_CMP (ld_canon_sysroot, realname) == 0;
    }

  free (realname);
  return result;
}

/* Adds NAME to the library search path.
   Makes a copy of NAME using xmalloc().  */

void
ldfile_add_library_path (const char *name, bool cmdline)
{
  search_dirs_type *new_dirs;

  if (!cmdline && config.only_cmd_line_lib_dirs)
    return;

  new_dirs = (search_dirs_type *) xmalloc (sizeof (search_dirs_type));
  new_dirs->next = NULL;
  new_dirs->cmdline = cmdline;
  *search_tail_ptr = new_dirs;
  search_tail_ptr = &new_dirs->next;

  /* If a directory is marked as honoring sysroot, prepend the sysroot path
     now.  */
  if (name[0] == '=')
    new_dirs->name = concat (ld_sysroot, name + 1, (const char *) NULL);
  else if (startswith (name, "$SYSROOT"))
    new_dirs->name = concat (ld_sysroot, name + strlen ("$SYSROOT"), (const char *) NULL);
  else
    new_dirs->name = xstrdup (name);
}

/* Try to open a BFD for a lang_input_statement.  */

bool
ldfile_try_open_bfd (const char *attempt,
		     lang_input_statement_type *entry)
{
  entry->the_bfd = bfd_openr (attempt, entry->target);

  if (verbose)
    {
      if (entry->the_bfd == NULL)
	info_msg (_("attempt to open %s failed\n"), attempt);
      else
	info_msg (_("attempt to open %s succeeded\n"), attempt);
    }

  if (entry->the_bfd == NULL)
    {
      if (bfd_get_error () == bfd_error_invalid_target)
	einfo (_("%F%P: invalid BFD target `%s'\n"), entry->target);
      return false;
    }

  /* PR 30568: Do not track lto generated temporary object files.  */
#if BFD_SUPPORTS_PLUGINS
  if (!entry->flags.lto_output)
#endif
    track_dependency_files (attempt);

  /* Linker needs to decompress sections.  */
  entry->the_bfd->flags |= BFD_DECOMPRESS;

  /* This is a linker input BFD.  */
  entry->the_bfd->is_linker_input = 1;

#if BFD_SUPPORTS_PLUGINS
  if (entry->flags.lto_output)
    entry->the_bfd->lto_output = 1;
#endif

  /* If we are searching for this file, see if the architecture is
     compatible with the output file.  If it isn't, keep searching.
     If we can't open the file as an object file, stop the search
     here.  If we are statically linking, ensure that we don't link
     a dynamic object.

     In the code below, it's OK to exit early if the check fails,
     closing the checked BFD and returning false, but if the BFD
     checks out compatible, do not exit early returning true, or
     the plugins will not get a chance to claim the file.  */

  if (entry->flags.search_dirs || !entry->flags.dynamic)
    {
      bfd *check;

      if (bfd_check_format (entry->the_bfd, bfd_archive))
	check = bfd_openr_next_archived_file (entry->the_bfd, NULL);
      else
	check = entry->the_bfd;

      if (check != NULL)
	{
	  if (!bfd_check_format (check, bfd_object))
	    {
	      if (check == entry->the_bfd
		  && entry->flags.search_dirs
		  && bfd_get_error () == bfd_error_file_not_recognized
		  && !ldemul_unrecognized_file (entry))
		{
		  int token, skip = 0;
		  char *arg, *arg1, *arg2, *arg3;
		  extern FILE *yyin;

		  /* Try to interpret the file as a linker script.  */
		  ldfile_open_command_file (attempt);

		  ldfile_assumed_script = true;
		  parser_input = input_selected;
		  ldlex_script ();
		  token = INPUT_SCRIPT;
		  while (token != 0)
		    {
		      switch (token)
			{
			case OUTPUT_FORMAT:
			  if ((token = yylex ()) != '(')
			    continue;
			  if ((token = yylex ()) != NAME)
			    continue;
			  arg1 = yylval.name;
			  arg2 = NULL;
			  arg3 = NULL;
			  token = yylex ();
			  if (token == ',')
			    {
			      if ((token = yylex ()) != NAME)
				{
				  free (arg1);
				  continue;
				}
			      arg2 = yylval.name;
			      if ((token = yylex ()) != ','
				  || (token = yylex ()) != NAME)
				{
				  free (arg1);
				  free (arg2);
				  continue;
				}
			      arg3 = yylval.name;
			      token = yylex ();
			    }
			  if (token == ')')
			    {
			      switch (command_line.endian)
				{
				default:
				case ENDIAN_UNSET:
				  arg = arg1; break;
				case ENDIAN_BIG:
				  arg = arg2 ? arg2 : arg1; break;
				case ENDIAN_LITTLE:
				  arg = arg3 ? arg3 : arg1; break;
				}
			      if (strcmp (arg, lang_get_output_target ()) != 0)
				skip = 1;
			    }
			  free (arg1);
			  free (arg2);
			  free (arg3);
			  break;
			case NAME:
			case LNAME:
			case VERS_IDENTIFIER:
			case VERS_TAG:
			  free (yylval.name);
			  break;
			case INT:
			  free (yylval.bigint.str);
			  break;
			}
		      token = yylex ();
		    }
		  ldlex_popstate ();
		  ldfile_assumed_script = false;
		  fclose (yyin);
		  yyin = NULL;
		  if (skip)
		    {
		      if (command_line.warn_search_mismatch)
			einfo (_("%P: skipping incompatible %s "
				 "when searching for %s\n"),
			       attempt, entry->local_sym_name);
		      bfd_close (entry->the_bfd);
		      entry->the_bfd = NULL;
		      return false;
		    }
		}
	      goto success;
	    }

	  if (!entry->flags.dynamic && (entry->the_bfd->flags & DYNAMIC) != 0)
	    {
	      einfo (_("%F%P: attempted static link of dynamic object `%s'\n"),
		     attempt);
	      bfd_close (entry->the_bfd);
	      entry->the_bfd = NULL;
	      return false;
	    }

	  if (entry->flags.search_dirs
	      && !bfd_arch_get_compatible (check, link_info.output_bfd,
					   command_line.accept_unknown_input_arch)
	      /* XCOFF archives can have 32 and 64 bit objects.  */
	      && !(bfd_get_flavour (check) == bfd_target_xcoff_flavour
		   && (bfd_get_flavour (link_info.output_bfd)
		       == bfd_target_xcoff_flavour)
		   && bfd_check_format (entry->the_bfd, bfd_archive)))
	    {
	      if (command_line.warn_search_mismatch)
		einfo (_("%P: skipping incompatible %s "
			 "when searching for %s\n"),
		       attempt, entry->local_sym_name);
	      bfd_close (entry->the_bfd);
	      entry->the_bfd = NULL;
	      return false;
	    }
	}
    }
 success:
#if BFD_SUPPORTS_PLUGINS
  /* If plugins are active, they get first chance to claim
     any successfully-opened input file.  We skip archives
     here; the plugin wants us to offer it the individual
     members when we enumerate them, not the whole file.  We
     also ignore corefiles, because that's just weird.  It is
     a needed side-effect of calling  bfd_check_format with
     bfd_object that it sets the bfd's arch and mach, which
     will be needed when and if we want to bfd_create a new
     one using this one as a template.  */
  if (link_info.lto_plugin_active
      && !no_more_claiming
      && bfd_check_format (entry->the_bfd, bfd_object))
    plugin_maybe_claim (entry);
#endif /* BFD_SUPPORTS_PLUGINS */

  /* It opened OK, the format checked out, and the plugins have had
     their chance to claim it, so this is success.  */
  return true;
}

/* Search for and open the file specified by ENTRY.  If it is an
   archive, use ARCH, LIB and SUFFIX to modify the file name.  */

bool
ldfile_open_file_search (const char *arch,
			 lang_input_statement_type *entry,
			 const char *lib,
			 const char *suffix)
{
  search_dirs_type *search;

  /* If this is not an archive, try to open it in the current
     directory first.  */
  if (!entry->flags.maybe_archive)
    {
      if (entry->flags.sysrooted && IS_ABSOLUTE_PATH (entry->filename))
	{
	  char *name = concat (ld_sysroot, entry->filename,
			       (const char *) NULL);
	  if (ldfile_try_open_bfd (name, entry))
	    {
	      entry->filename = name;
	      return true;
	    }
	  free (name);
	}
      else if (ldfile_try_open_bfd (entry->filename, entry))
	return true;

      if (IS_ABSOLUTE_PATH (entry->filename))
	return false;
    }

  for (search = search_head; search != NULL; search = search->next)
    {
      char *string;

      if (entry->flags.dynamic && !bfd_link_relocatable (&link_info))
	{
	  if (ldemul_open_dynamic_archive (arch, search, entry))
	    return true;
	}

      if (entry->flags.maybe_archive && !entry->flags.full_name_provided)
	string = concat (search->name, slash, lib, entry->filename,
			 arch, suffix, (const char *) NULL);
      else
	string = concat (search->name, slash, entry->filename,
			 (const char *) 0);

      if (ldfile_try_open_bfd (string, entry))
	{
	  entry->filename = string;
	  return true;
	}

      free (string);
    }

  return false;
}

/* Open the input file specified by ENTRY.
   PR 4437: Do not stop on the first missing file, but
   continue processing other input files in case there
   are more errors to report.  */

void
ldfile_open_file (lang_input_statement_type *entry)
{
  if (entry->the_bfd != NULL)
    return;

  if (!entry->flags.search_dirs)
    {
      if (ldfile_try_open_bfd (entry->filename, entry))
	return;

      if (filename_cmp (entry->filename, entry->local_sym_name) != 0)
	einfo (_("%P: cannot find %s (%s): %E\n"),
	       entry->filename, entry->local_sym_name);
      else
	einfo (_("%P: cannot find %s: %E\n"), entry->local_sym_name);

      entry->flags.missing_file = true;
      input_flags.missing_file = true;
    }
  else
    {
      search_arch_type *arch;
      bool found = false;

      /* If extra_search_path is set, entry->filename is a relative path.
	 Search the directory of the current linker script before searching
	 other paths. */
      if (entry->extra_search_path)
	{
	  char *path = concat (entry->extra_search_path, slash, entry->filename,
			       (const char *)0);
	  if (ldfile_try_open_bfd (path, entry))
	    {
	      entry->filename = path;
	      entry->flags.search_dirs = false;
	      return;
	    }

	  free (path);
	}

      /* Try to open <filename><suffix> or lib<filename><suffix>.a.  */
      for (arch = search_arch_head; arch != NULL; arch = arch->next)
	{
	  found = ldfile_open_file_search (arch->name, entry, "lib", ".a");
	  if (found)
	    break;
#ifdef VMS
	  found = ldfile_open_file_search (arch->name, entry, ":lib", ".a");
	  if (found)
	    break;
#endif
	  found = ldemul_find_potential_libraries (arch->name, entry);
	  if (found)
	    break;
	}

      /* If we have found the file, we don't need to search directories
	 again.  */
      if (found)
	entry->flags.search_dirs = false;
      else
	{
	  if (entry->flags.sysrooted
	       && ld_sysroot
	       && IS_ABSOLUTE_PATH (entry->local_sym_name))
	    einfo (_("%P: cannot find %s inside %s\n"),
		   entry->local_sym_name, ld_sysroot);
#if SUPPORT_ERROR_HANDLING_SCRIPT
	  else if (error_handling_script != NULL)
	    {
	      char *        argv[4];
	      const char *  res;
	      int           status, err;

	      argv[0] = error_handling_script;
	      argv[1] = "missing-lib";
	      argv[2] = (char *) entry->local_sym_name;
	      argv[3] = NULL;
      
	      if (verbose)
		einfo (_("%P: About to run error handling script '%s' with arguments: '%s' '%s'\n"),
		       argv[0], argv[1], argv[2]);

	      res = pex_one (PEX_SEARCH, error_handling_script, argv,
			     N_("error handling script"),
			     NULL /* Send stdout to random, temp file.  */,
			     NULL /* Write to stderr.  */,
			     &status, &err);
	      if (res != NULL)
		{
		  einfo (_("%P: Failed to run error handling script '%s', reason: "),
			 error_handling_script);
		  /* FIXME: We assume here that errrno == err.  */
		  perror (res);
		}
	      else /* We ignore the return status of the script
		      and always print the error message.  */
		einfo (_("%P: cannot find %s: %E\n"), entry->local_sym_name);
	    }
#endif
	  else
	    einfo (_("%P: cannot find %s: %E\n"), entry->local_sym_name);

	  /* PR 25747: Be kind to users who forgot to add the
	     "lib" prefix to their library when it was created.  */
	  for (arch = search_arch_head; arch != NULL; arch = arch->next)
	    {
	      if (ldfile_open_file_search (arch->name, entry, "", ".a"))
		{
		  const char * base = lbasename (entry->filename);

		  einfo (_("%P: note to link with %s use -l:%s or rename it to lib%s\n"),
			 entry->filename, base, base);
		  bfd_close (entry->the_bfd);
		  entry->the_bfd = NULL;
		  break;
		}
	    }

	  entry->flags.missing_file = true;
	  input_flags.missing_file = true;
	}
    }
}

/* Try to open NAME.  */

static FILE *
try_open (const char *name, bool *sysrooted)
{
  FILE *result;

  result = fopen (name, "r");

  if (result != NULL)
    *sysrooted = is_sysrooted_pathname (name);

  if (verbose)
    {
      if (result == NULL)
	info_msg (_("cannot find script file %s\n"), name);
      else
	info_msg (_("opened script file %s\n"), name);
    }

  return result;
}

/* Return TRUE iff directory DIR contains an "ldscripts" subdirectory.  */

static bool
check_for_scripts_dir (char *dir)
{
  char *buf;
  struct stat s;
  bool res;

  buf = concat (dir, "/ldscripts", (const char *) NULL);
  res = stat (buf, &s) == 0 && S_ISDIR (s.st_mode);
  free (buf);
  return res;
}

/* Return the default directory for finding script files.
   We look for the "ldscripts" directory in:

   SCRIPTDIR (passed from Makefile)
	     (adjusted according to the current location of the binary)
   the dir where this program is (for using it from the build tree).  */

static char *
find_scripts_dir (void)
{
  char *dir;

  dir = make_relative_prefix (program_name, BINDIR, SCRIPTDIR);
  if (dir)
    {
      if (check_for_scripts_dir (dir))
	return dir;
      free (dir);
    }

  dir = make_relative_prefix (program_name, TOOLBINDIR, SCRIPTDIR);
  if (dir)
    {
      if (check_for_scripts_dir (dir))
	return dir;
      free (dir);
    }

  /* Look for "ldscripts" in the dir where our binary is.  */
  dir = make_relative_prefix (program_name, ".", ".");
  if (dir)
    {
      if (check_for_scripts_dir (dir))
	return dir;
      free (dir);
    }

  return NULL;
}

/* If DEFAULT_ONLY is false, try to open NAME; if that fails, look for
   it in directories specified with -L, then in the default script
   directory.  If DEFAULT_ONLY is true, the search is restricted to
   the default script location.  */

static FILE *
ldfile_find_command_file (const char *name,
			  bool default_only,
			  bool *sysrooted)
{
  search_dirs_type *search;
  FILE *result = NULL;
  char *path;
  static search_dirs_type *script_search;

  if (!default_only)
    {
      /* First try raw name.  */
      result = try_open (name, sysrooted);
      if (result != NULL)
	return result;
    }

  if (!script_search)
    {
      char *script_dir = find_scripts_dir ();
      if (script_dir)
	{
	  search_dirs_type **save_tail_ptr = search_tail_ptr;
	  search_tail_ptr = &script_search;
	  ldfile_add_library_path (script_dir, true);
	  search_tail_ptr = save_tail_ptr;
	}
    }

  /* Temporarily append script_search to the path list so that the
     paths specified with -L will be searched first.  */
  *search_tail_ptr = script_search;

  /* Try now prefixes.  */
  for (search = default_only ? script_search : search_head;
       search != NULL;
       search = search->next)
    {
      path = concat (search->name, slash, name, (const char *) NULL);
      result = try_open (path, sysrooted);
      free (path);
      if (result)
	break;
    }

  /* Restore the original path list.  */
  *search_tail_ptr = NULL;

  return result;
}

enum script_open_style {
  script_nonT,
  script_T,
  script_defaultT
};

struct script_name_list
{
  struct script_name_list *next;
  enum script_open_style open_how;
  char name[1];
};

/* Open command file NAME.  */

static void
ldfile_open_command_file_1 (const char *name, enum script_open_style open_how)
{
  FILE *ldlex_input_stack;
  bool sysrooted;
  static struct script_name_list *processed_scripts = NULL;
  struct script_name_list *script;
  size_t len;

  /* PR 24576: Catch the case where the user has accidentally included
     the same linker script twice.  */
  for (script = processed_scripts; script != NULL; script = script->next)
    {
      if ((open_how != script_nonT || script->open_how != script_nonT)
	  && strcmp (name, script->name) == 0)
	{
	  einfo (_("%F%P: error: linker script file '%s'"
		   " appears multiple times\n"), name);
	  return;
	}
    }

  /* FIXME: This memory is never freed, but that should not really matter.
     It will be released when the linker exits, and it is unlikely to ever
     be more than a few tens of bytes.  */
  len = strlen (name);
  script = xmalloc (sizeof (*script) + len);
  script->next = processed_scripts;
  script->open_how = open_how;
  memcpy (script->name, name, len + 1);
  processed_scripts = script;

  ldlex_input_stack = ldfile_find_command_file (name,
						open_how == script_defaultT,
						&sysrooted);
  if (ldlex_input_stack == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      einfo (_("%F%P: cannot open linker script file %s: %E\n"), name);
      return;
    }

  track_dependency_files (name);

  lex_push_file (ldlex_input_stack, name, sysrooted);

  lineno = 1;

  saved_script_handle = ldlex_input_stack;
}

/* Open command file NAME in the current directory, -L directories,
   the default script location, in that order.  */

void
ldfile_open_command_file (const char *name)
{
  ldfile_open_command_file_1 (name, script_nonT);
}

void
ldfile_open_script_file (const char *name)
{
  ldfile_open_command_file_1 (name, script_T);
}

/* Open command file NAME at the default script location.  */

void
ldfile_open_default_command_file (const char *name)
{
  ldfile_open_command_file_1 (name, script_defaultT);
}

void
ldfile_add_arch (const char *in_name)
{
  char *name = xstrdup (in_name);
  search_arch_type *new_arch
    = (search_arch_type *) xmalloc (sizeof (search_arch_type));

  ldfile_output_machine_name = in_name;

  new_arch->name = name;
  new_arch->next = NULL;
  while (*name)
    {
      *name = TOLOWER (*name);
      name++;
    }
  *search_arch_tail_ptr = new_arch;
  search_arch_tail_ptr = &new_arch->next;

}

/* Set the output architecture.  */

void
ldfile_set_output_arch (const char *string, enum bfd_architecture defarch)
{
  const bfd_arch_info_type *arch = bfd_scan_arch (string);

  if (arch)
    {
      ldfile_output_architecture = arch->arch;
      ldfile_output_machine = arch->mach;
      ldfile_output_machine_name = arch->printable_name;
    }
  else if (defarch != bfd_arch_unknown)
    ldfile_output_architecture = defarch;
  else
    einfo (_("%F%P: cannot represent machine `%s'\n"), string);
}
