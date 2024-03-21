/* Test plugin for the GNU linker.  Check non-object IR file and calling
   release_input_file from onclaim_file.
   Copyright (C) 2015-2023 Free Software Foundation, Inc.

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
#if BFD_SUPPORTS_PLUGINS
#include "plugin-api.h"
#include "filenames.h"
/* For ARRAY_SIZE macro only - we don't link the library itself.  */
#include "libiberty.h"

extern enum ld_plugin_status onload (struct ld_plugin_tv *tv);
static enum ld_plugin_status onclaim_file (const struct ld_plugin_input_file *file,
				int *claimed);
static enum ld_plugin_status onall_symbols_read (void);
static enum ld_plugin_status oncleanup (void);

/* Helper for calling plugin api message function.  */
#define TV_MESSAGE if (tv_message) (*tv_message)

/* Struct for recording files to claim / files claimed.  */
typedef struct claim_file
{
  struct claim_file *next;
  struct ld_plugin_input_file file;
  bool claimed;
  struct ld_plugin_symbol *symbols;
  int n_syms_allocated;
  int n_syms_used;
} claim_file_t;

/* Types of things that can be added at all symbols read time.  */
typedef enum addfile_enum
{
  ADD_FILE,
  ADD_LIB,
  ADD_DIR
} addfile_enum_t;

/* Struct for recording files to add to final link.  */
typedef struct add_file
{
  struct add_file *next;
  const char *name;
  addfile_enum_t type;
} add_file_t;

/* Helper macro for defining array of transfer vector tags and names.  */
#define ADDENTRY(tag) { tag, #tag }

/* Struct for looking up human-readable versions of tag names.  */
typedef struct tag_name
{
  enum ld_plugin_tag tag;
  const char *name;
} tag_name_t;

/* Array of all known tags and their names.  */
static const tag_name_t tag_names[] =
{
  ADDENTRY(LDPT_NULL),
  ADDENTRY(LDPT_API_VERSION),
  ADDENTRY(LDPT_GOLD_VERSION),
  ADDENTRY(LDPT_LINKER_OUTPUT),
  ADDENTRY(LDPT_OPTION),
  ADDENTRY(LDPT_REGISTER_CLAIM_FILE_HOOK),
  ADDENTRY(LDPT_REGISTER_CLAIM_FILE_HOOK_V2),
  ADDENTRY(LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK),
  ADDENTRY(LDPT_REGISTER_CLEANUP_HOOK),
  ADDENTRY(LDPT_ADD_SYMBOLS),
  ADDENTRY(LDPT_GET_SYMBOLS),
  ADDENTRY(LDPT_GET_SYMBOLS_V2),
  ADDENTRY(LDPT_ADD_INPUT_FILE),
  ADDENTRY(LDPT_MESSAGE),
  ADDENTRY(LDPT_GET_INPUT_FILE),
  ADDENTRY(LDPT_GET_VIEW),
  ADDENTRY(LDPT_RELEASE_INPUT_FILE),
  ADDENTRY(LDPT_ADD_INPUT_LIBRARY),
  ADDENTRY(LDPT_OUTPUT_NAME),
  ADDENTRY(LDPT_SET_EXTRA_LIBRARY_PATH),
  ADDENTRY(LDPT_GNU_LD_VERSION)
};

/* Function pointers to cache hooks passed at onload time.  */
static ld_plugin_register_claim_file tv_register_claim_file = 0;
static ld_plugin_register_claim_file_v2 tv_register_claim_file_v2 = 0;
static ld_plugin_register_all_symbols_read tv_register_all_symbols_read = 0;
static ld_plugin_register_cleanup tv_register_cleanup = 0;
static ld_plugin_add_symbols tv_add_symbols = 0;
static ld_plugin_get_symbols tv_get_symbols = 0;
static ld_plugin_get_symbols tv_get_symbols_v2 = 0;
static ld_plugin_add_input_file tv_add_input_file = 0;
static ld_plugin_message tv_message = 0;
static ld_plugin_get_input_file tv_get_input_file = 0;
static ld_plugin_get_view tv_get_view = 0;
static ld_plugin_release_input_file tv_release_input_file = 0;
static ld_plugin_add_input_library tv_add_input_library = 0;
static ld_plugin_set_extra_library_path tv_set_extra_library_path = 0;

/* Other cached info from the transfer vector.  */
static enum ld_plugin_output_file_type linker_output;
static const char *output_name;

/* Behaviour control flags set by plugin options.  */
static enum ld_plugin_status onload_ret = LDPS_OK;
static enum ld_plugin_status claim_file_ret = LDPS_OK;
static enum ld_plugin_status all_symbols_read_ret = LDPS_OK;
static enum ld_plugin_status cleanup_ret = LDPS_OK;
static bool register_claimfile_hook = true;
static bool register_allsymbolsread_hook = false;
static bool register_cleanup_hook = false;
static bool dumpresolutions = false;

/* The master list of all claimable/claimed files.  */
static claim_file_t *claimfiles_list = NULL;

/* We keep a tail pointer for easy linking on the end.  */
static claim_file_t **claimfiles_tail_chain_ptr = &claimfiles_list;

/* The last claimed file added to the list, for receiving syms.  */
static claim_file_t *last_claimfile = NULL;

/* The master list of all files to add to the final link.  */
static add_file_t *addfiles_list = NULL;

/* We keep a tail pointer for easy linking on the end.  */
static add_file_t **addfiles_tail_chain_ptr = &addfiles_list;

/* Add a new claimfile on the end of the chain.  */
static enum ld_plugin_status
record_claim_file (const char *file, off_t filesize)
{
  claim_file_t *newfile;

  newfile = malloc (sizeof *newfile);
  if (!newfile)
    return LDPS_ERR;
  memset (newfile, 0, sizeof *newfile);
  /* Only setup for now is remembering the name to look for.  */
  newfile->file.name = file;
  newfile->file.filesize = filesize;
  /* Chain it on the end of the list.  */
  *claimfiles_tail_chain_ptr = newfile;
  claimfiles_tail_chain_ptr = &newfile->next;
  /* Record it as active for receiving symbols to register.  */
  last_claimfile = newfile;
  return LDPS_OK;
}

/* Add a new addfile on the end of the chain.  */
static enum ld_plugin_status
record_add_file (const char *file, addfile_enum_t type)
{
  add_file_t *newfile;

  newfile = malloc (sizeof *newfile);
  if (!newfile)
    return LDPS_ERR;
  newfile->next = NULL;
  newfile->name = file;
  newfile->type = type;
  /* Chain it on the end of the list.  */
  *addfiles_tail_chain_ptr = newfile;
  addfiles_tail_chain_ptr = &newfile->next;
  return LDPS_OK;
}

/* Parse a command-line argument string into a symbol definition.
   Symbol-strings follow the colon-separated format:
	NAME:VERSION:def:vis:size:COMDATKEY
   where the fields in capitals are strings and those in lower
   case are integers.  We don't allow to specify a resolution as
   doing so is not meaningful when calling the add symbols hook.  */
static enum ld_plugin_status
parse_symdefstr (const char *str, struct ld_plugin_symbol *sym)
{
  int n;
  long long size;
  const char *colon1, *colon2, *colon5;

  /* Locate the colons separating the first two strings.  */
  colon1 = strchr (str, ':');
  if (!colon1)
    return LDPS_ERR;
  colon2 = strchr (colon1+1, ':');
  if (!colon2)
    return LDPS_ERR;
  /* Name must not be empty (version may be).  */
  if (colon1 == str)
    return LDPS_ERR;

  /* The fifth colon and trailing comdat key string are optional,
     but the intermediate ones must all be present.  */
  colon5 = strchr (colon2+1, ':');	/* Actually only third so far.  */
  if (!colon5)
    return LDPS_ERR;
  colon5 = strchr (colon5+1, ':');	/* Hopefully fourth now.  */
  if (!colon5)
    return LDPS_ERR;
  colon5 = strchr (colon5+1, ':');	/* Optional fifth now.  */

  /* Finally we'll use sscanf to parse the numeric fields, then
     we'll split out the strings which we need to allocate separate
     storage for anyway so that we can add nul termination.  */
  n = sscanf (colon2 + 1, "%hhi:%i:%lli", &sym->def, &sym->visibility, &size);
  if (n != 3)
    return LDPS_ERR;

  /* Parsed successfully, so allocate strings and fill out fields.  */
  sym->size = size;
  sym->unused = 0;
  sym->section_kind = 0;
  sym->symbol_type = 0;
  sym->resolution = LDPR_UNKNOWN;
  sym->name = malloc (colon1 - str + 1);
  if (!sym->name)
    return LDPS_ERR;
  memcpy (sym->name, str, colon1 - str);
  sym->name[colon1 - str] = '\0';
  if (colon2 > (colon1 + 1))
    {
      sym->version = malloc (colon2 - colon1);
      if (!sym->version)
	return LDPS_ERR;
      memcpy (sym->version, colon1 + 1, colon2 - (colon1 + 1));
      sym->version[colon2 - (colon1 + 1)] = '\0';
    }
  else
    sym->version = NULL;
  if (colon5 && colon5[1])
    {
      sym->comdat_key = malloc (strlen (colon5 + 1) + 1);
      if (!sym->comdat_key)
	return LDPS_ERR;
      strcpy (sym->comdat_key, colon5 + 1);
    }
  else
    sym->comdat_key = 0;
  return LDPS_OK;
}

/* Record a symbol to be added for the last-added claimfile.  */
static enum ld_plugin_status
record_claimed_file_symbol (const char *symdefstr)
{
  struct ld_plugin_symbol sym;

  /* Can't add symbols except as belonging to claimed files.  */
  if (!last_claimfile)
    return LDPS_ERR;

  /* If string doesn't parse correctly, give an error.  */
  if (parse_symdefstr (symdefstr, &sym) != LDPS_OK)
    return LDPS_ERR;

  /* Check for enough space, resize array if needed, and add it.  */
  if (last_claimfile->n_syms_allocated == last_claimfile->n_syms_used)
    {
      int new_n_syms = last_claimfile->n_syms_allocated
			? 2 * last_claimfile->n_syms_allocated
			: 10;
      last_claimfile->symbols = realloc (last_claimfile->symbols,
			new_n_syms * sizeof *last_claimfile->symbols);
      if (!last_claimfile->symbols)
	return LDPS_ERR;
      last_claimfile->n_syms_allocated = new_n_syms;
    }
  last_claimfile->symbols[last_claimfile->n_syms_used++] = sym;

  return LDPS_OK;
}

/* Records the status to return from one of the registered hooks.  */
static enum ld_plugin_status
set_ret_val (const char *whichval, enum ld_plugin_status retval)
{
  if (!strcmp ("onload", whichval))
    onload_ret = retval;
  else if (!strcmp ("claimfile", whichval))
    claim_file_ret = retval;
  else if (!strcmp ("allsymbolsread", whichval))
    all_symbols_read_ret = retval;
  else if (!strcmp ("cleanup", whichval))
    cleanup_ret = retval;
  else
    return LDPS_ERR;
  return LDPS_OK;
}

/* Records hooks which should be registered.  */
static enum ld_plugin_status
set_register_hook (const char *whichhook, bool yesno)
{
  if (!strcmp ("claimfile", whichhook))
    register_claimfile_hook = yesno;
  else if (!strcmp ("allsymbolsread", whichhook))
    register_allsymbolsread_hook = yesno;
  else if (!strcmp ("cleanup", whichhook))
    register_cleanup_hook = yesno;
  else
    return LDPS_ERR;
  return LDPS_OK;
}

/* Determine type of plugin option and pass to individual parsers.  */
static enum ld_plugin_status
parse_option (const char *opt)
{
  if (!strncmp ("fail", opt, 4))
    return set_ret_val (opt + 4, LDPS_ERR);
  else if (!strncmp ("pass", opt, 4))
    return set_ret_val (opt + 4, LDPS_OK);
  else if (!strncmp ("register", opt, 8))
    return set_register_hook (opt + 8, true);
  else if (!strncmp ("noregister", opt, 10))
    return set_register_hook (opt + 10, false);
  else if (!strncmp ("claim:", opt, 6))
    return record_claim_file (opt + 6, 0);
  else if (!strncmp ("sym:", opt, 4))
    return record_claimed_file_symbol (opt + 4);
  else if (!strncmp ("add:", opt, 4))
    return record_add_file (opt + 4, ADD_FILE);
  else if (!strncmp ("lib:", opt, 4))
    return record_add_file (opt + 4, ADD_LIB);
  else if (!strncmp ("dir:", opt, 4))
    return record_add_file (opt + 4, ADD_DIR);
  else if (!strcmp ("dumpresolutions", opt))
    dumpresolutions = true;
  else
    return LDPS_ERR;
  return LDPS_OK;
}

/* Handle/record information received in a transfer vector entry.  */
static enum ld_plugin_status
parse_tv_tag (struct ld_plugin_tv *tv)
{
#define SETVAR(x) x = tv->tv_u.x
  switch (tv->tv_tag)
    {
      case LDPT_OPTION:
	return parse_option (tv->tv_u.tv_string);
      case LDPT_NULL:
      case LDPT_GOLD_VERSION:
      case LDPT_GNU_LD_VERSION:
      case LDPT_API_VERSION:
      default:
	break;
      case LDPT_OUTPUT_NAME:
	output_name = tv->tv_u.tv_string;
	break;
      case LDPT_LINKER_OUTPUT:
	linker_output = tv->tv_u.tv_val;
	break;
      case LDPT_REGISTER_CLAIM_FILE_HOOK:
	SETVAR(tv_register_claim_file);
	break;
      case LDPT_REGISTER_CLAIM_FILE_HOOK_V2:
	SETVAR(tv_register_claim_file_v2);
	break;
      case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
	SETVAR(tv_register_all_symbols_read);
	break;
      case LDPT_REGISTER_CLEANUP_HOOK:
	SETVAR(tv_register_cleanup);
	break;
      case LDPT_ADD_SYMBOLS:
	SETVAR(tv_add_symbols);
	break;
      case LDPT_GET_SYMBOLS:
	SETVAR(tv_get_symbols);
	break;
      case LDPT_GET_SYMBOLS_V2:
	tv_get_symbols_v2 = tv->tv_u.tv_get_symbols;
	break;
      case LDPT_ADD_INPUT_FILE:
	SETVAR(tv_add_input_file);
	break;
      case LDPT_MESSAGE:
	SETVAR(tv_message);
	break;
      case LDPT_GET_INPUT_FILE:
	SETVAR(tv_get_input_file);
	break;
      case LDPT_GET_VIEW:
	SETVAR(tv_get_view);
	break;
      case LDPT_RELEASE_INPUT_FILE:
	SETVAR(tv_release_input_file);
	break;
      case LDPT_ADD_INPUT_LIBRARY:
	SETVAR(tv_add_input_library);
	break;
      case LDPT_SET_EXTRA_LIBRARY_PATH:
	SETVAR(tv_set_extra_library_path);
	break;
    }
#undef SETVAR
  return LDPS_OK;
}

/* Standard plugin API entry point.  */
enum ld_plugin_status
onload (struct ld_plugin_tv *tv)
{
  enum ld_plugin_status rv;

  /* This plugin does nothing but dump the tv array.  It would
     be an error if this function was called without one.  */
  if (!tv)
    return LDPS_ERR;

  /* First entry should always be LDPT_MESSAGE, letting us get
     hold of it easily so we can send output straight away.  */
  if (tv[0].tv_tag == LDPT_MESSAGE)
    tv_message = tv[0].tv_u.tv_message;

  do
    if ((rv = parse_tv_tag (tv)) != LDPS_OK)
      return rv;
  while ((tv++)->tv_tag != LDPT_NULL);

  /* Register hooks only if instructed by options.  */
  if (register_claimfile_hook)
    {
      if (!tv_register_claim_file)
	{
	  TV_MESSAGE (LDPL_FATAL, "No register_claim_file hook");
	  fflush (NULL);
	  return LDPS_ERR;
	}
      (*tv_register_claim_file) (onclaim_file);
    }
  if (register_allsymbolsread_hook)
    {
      if (!tv_register_all_symbols_read)
	{
	  TV_MESSAGE (LDPL_FATAL, "No register_all_symbols_read hook");
	  fflush (NULL);
	  return LDPS_ERR;
	}
      (*tv_register_all_symbols_read) (onall_symbols_read);
    }
  if (register_cleanup_hook)
    {
      if (!tv_register_cleanup)
	{
	  TV_MESSAGE (LDPL_FATAL, "No register_cleanup hook");
	  fflush (NULL);
	  return LDPS_ERR;
	}
      (*tv_register_cleanup) (oncleanup);
    }

  /* Claim testsuite/ld-plugin/func.c, standalone or in a library.  Its
     size must be SIZE_OF_FUNC_C bytes.  */
#define SIZE_OF_FUNC_C	248
  if (onload_ret == LDPS_OK
      && (record_claim_file ("func.c", SIZE_OF_FUNC_C) != LDPS_OK
	  || record_claimed_file_symbol ("func::0:0:0") != LDPS_OK
	  || record_claimed_file_symbol ("_func::0:0:0") != LDPS_OK
	  || record_claim_file ("libfunc.a", SIZE_OF_FUNC_C) != LDPS_OK
	  || record_claimed_file_symbol ("func::0:0:0") != LDPS_OK
	  || record_claimed_file_symbol ("_func::0:0:0") != LDPS_OK))
    onload_ret = LDPS_ERR;

  return onload_ret;
}

char *
xstrdup (const char *s)
{
  size_t len = strlen (s) + 1;
  char *ret = malloc (len + 1);
  return (char *) memcpy (ret, s, len);
}

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
onclaim_file (const struct ld_plugin_input_file *file, int *claimed)
{
  /* Let's see if we want to claim this file.  */
  claim_file_t *claimfile = claimfiles_list;
  size_t len = strlen (file->name);
  char *name = xstrdup (file->name);
  char *p = name + len;
  bool islib;

  /* Only match the file name without the directory part.  */
  islib = *p == 'a' && *(p - 1) == '.';
  for (; p != name; p--)
    if (IS_DIR_SEPARATOR (*p))
      {
	p++;
	break;
      }

  while (claimfile)
    {
      /* Claim the file only if the file name and size match and don't
	 match the whole library.  */
      if (!strcmp (p, claimfile->file.name)
	  && claimfile->file.filesize == file->filesize
	  && (!islib || file->offset != 0))
	break;
      claimfile = claimfile->next;
    }

  free (name);

  /* If we decided to claim it, record that fact, and add any symbols
     that were defined for it by plugin options.  */
  *claimed = (claimfile != 0);
  if (claimfile)
    {
      char buffer[30];
      int fd;

      TV_MESSAGE (LDPL_INFO, "Claimed: %s [@%ld/%ld]", file->name,
		  (long)file->offset, (long)file->filesize);

      claimfile->claimed = true;
      claimfile->file = *file;
      if (claimfile->n_syms_used && !tv_add_symbols)
	claim_file_ret = LDPS_ERR;
      else if (claimfile->n_syms_used)
	claim_file_ret = (*tv_add_symbols) (claimfile->file.handle,
					    claimfile->n_syms_used,
					    claimfile->symbols);

      fd = claimfile->file.fd;
      name = xstrdup (claimfile->file.name);
      claim_file_ret = tv_release_input_file (claimfile->file.handle);
      if (claim_file_ret != LDPS_OK)
	{
	  free (name);
	  return claim_file_ret;
	}
      if (read (fd, buffer, sizeof (buffer)) >= 0)
	{
	  claim_file_ret = LDPS_ERR;
	  TV_MESSAGE (LDPL_FATAL, "Unreleased file descriptor on: %s", name);
	}
      free (name);
    }

  return claim_file_ret;
}

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
onall_symbols_read (void)
{
  static const char *resolutions[] =
    {
      "LDPR_UNKNOWN",
      "LDPR_UNDEF",
      "LDPR_PREVAILING_DEF",
      "LDPR_PREVAILING_DEF_IRONLY",
      "LDPR_PREEMPTED_REG",
      "LDPR_PREEMPTED_IR",
      "LDPR_RESOLVED_IR",
      "LDPR_RESOLVED_EXEC",
      "LDPR_RESOLVED_DYN",
      "LDPR_PREVAILING_DEF_IRONLY_EXP",
    };
  claim_file_t *claimfile = dumpresolutions ? claimfiles_list : NULL;
  add_file_t *addfile = addfiles_list;
  TV_MESSAGE (LDPL_INFO, "hook called: all symbols read.");
  for ( ; claimfile; claimfile = claimfile->next)
    {
      enum ld_plugin_status rv;
      int n;
      if (claimfile->n_syms_used && !tv_get_symbols_v2)
	return LDPS_ERR;
      else if (!claimfile->n_syms_used)
        continue;
      else if (!claimfile->file.handle)
        continue;
      rv = tv_get_symbols_v2 (claimfile->file.handle, claimfile->n_syms_used,
			      claimfile->symbols);
      if (rv != LDPS_OK)
	return rv;
      for (n = 0; n < claimfile->n_syms_used; n++)
	TV_MESSAGE (LDPL_INFO, "Sym: '%s%s%s' Resolution: %s",
		    claimfile->symbols[n].name,
		    claimfile->symbols[n].version ? "@" : "",
		    (claimfile->symbols[n].version
		     ? claimfile->symbols[n].version : ""),
		    resolutions[claimfile->symbols[n].resolution]);
    }
  for ( ; addfile ; addfile = addfile->next)
    {
      enum ld_plugin_status rv;
      if (addfile->type == ADD_LIB && tv_add_input_library)
	rv = (*tv_add_input_library) (addfile->name);
      else if (addfile->type == ADD_FILE && tv_add_input_file)
	rv = (*tv_add_input_file) (addfile->name);
      else if (addfile->type == ADD_DIR && tv_set_extra_library_path)
	rv = (*tv_set_extra_library_path) (addfile->name);
      else
	rv = LDPS_ERR;
      if (rv != LDPS_OK)
	return rv;
    }
  fflush (NULL);
  return all_symbols_read_ret;
}

/* Standard plugin API registerable hook.  */
static enum ld_plugin_status
oncleanup (void)
{
  TV_MESSAGE (LDPL_INFO, "hook called: cleanup.");
  fflush (NULL);
  return cleanup_ret;
}
#endif /* BFD_SUPPORTS_PLUGINS */
