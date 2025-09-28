/* mktemp - create temporary file or directory */

/*
   Copyright (C) 2019 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include "bashansi.h"

#include "loadables.h"

#define DEFAULT_PREFIX	"shtmp"

int
mktemp_builtin (list)
     WORD_LIST *list;
{
  WORD_LIST *l;
  int rval, opt, fd, mflags, base_mflags;
  int dflag, qflag, tflag, uflag, onetime;
  char *prefix, *varname, *filename, *template;
  SHELL_VAR *v;

  dflag = qflag = uflag = tflag = onetime = 0;
  prefix = varname = 0;
  rval = EXECUTION_SUCCESS;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "dqut:v:")) != -1)
    {
      switch (opt)
	{
	case 'd':
	  dflag = 1;
	  break;
	case 'q':
	  qflag = 1;
	  break;
	case 't':
	  tflag = 1;
	  prefix = list_optarg;
	  break;
	case 'u':
	  uflag = 1;
	  break;
	case 'v':
	  varname = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (varname)			/* check for validity, not readonly */
    {
      if (legal_identifier (varname) == 0)
	{
	  if (qflag == 0)
	    sh_invalidid (varname);
	  return (EXECUTION_FAILURE);
	}
      v = find_variable (varname);
      if (v && readonly_p (v))
	{
	  if (qflag == 0)
	    sh_readonly (varname);
	  return (EXECUTION_FAILURE);
	}
    }

  onetime = (list == 0);	/* once through the loop, $TMPDIR/prefix.XXXXXX */

  if (prefix == 0)
    prefix = DEFAULT_PREFIX;
  base_mflags = MT_USETMPDIR|MT_USERANDOM;	/* USERANDOM not strictly needed */

  while (list || onetime)
    {
      mflags = base_mflags;
      onetime = 0;
#if defined (USE_MKTEMP) && defined (USE_MKSTEMP)
      if (list)
	{
	  template = list->word->word;
	  mflags |= MT_TEMPLATE;
	}
#else
      /* This is sub-optimal. */
      if (list)
	{
	  /* Treat the basename as a prefix */
	  template = strrchr (list->word->word, '/');
	  if (template)
	    template++;
	  else
	    template = list->word->word;
	}
#endif
      else
	template = prefix;

      if (dflag)
	{
	  filename = sh_mktmpdir (template, mflags);
	  if (filename == 0)
	    {
	      if (qflag == 0)
		builtin_error ("%s: cannot create directory", template);
	      rval = EXECUTION_FAILURE;
	    }
	  else
	    {
	      if (uflag)
		rmdir (filename);
	      printf ("%s\n", filename);
	    }
	}
      else		/* filename */
	{
	  fd = sh_mktmpfd (template, mflags, &filename);
	  if (fd < 0)
	    {
	      if (qflag == 0)
		builtin_error ("%s: cannot create file", template);
	      rval = EXECUTION_FAILURE;
	    }
	  else
	    {
	      close (fd);
	      if (uflag)
		unlink (filename);
	      printf ("%s\n", filename);
	    }
	}

      /* Assign variable if requested */
      if (filename && varname)
	{
	  v = builtin_bind_variable (varname, filename, 0);
	  if (v == 0 || readonly_p (v) || noassign_p (v))
	    {
	      builtin_error ("%s: cannot set variable", varname);
	      rval = EXECUTION_FAILURE;
	    }
	}

      FREE (filename);

      if (list)
        list = list->next;
    }

  return (rval);
}

char *mktemp_doc[] = {
	"Make unique temporary file name",
	"",
	"Take each supplied filename template and overwrite a portion of it",
	"to create a filename, which is unique and may be used by the calling",
	"script. TEMPLATE is a string ending in some number of 'X's. If",
	"TEMPLATE is not supplied, shtmp.XXXXXX is used and $TMPDIR is used as",
	"the name of the containing directory. Files are created u+rw; directories",
	"are created u+rwx.",
	"",
	"Options, if supplied, have the following meanings:",
	"",
	"    -d    Create a directory instead of a file",
	"    -q    Do not print error messages about file creation failure",
	"    -t PREFIX Use PREFIX as the directory in which to create files",
	"    -u    Do not create anything; simply print a name",
	"    -v VAR    Store the generated name into shell variable VAR",
	"",
	"Any PREFIX supplied with -t is ignored if TEMPLATE is supplied.",
	"",
	"The return status is true if the file or directory was created successfully;",
	"false if an error occurs or VAR is invalid or readonly.",

	(char *)NULL
};

struct builtin mktemp_struct = {
	"mktemp",		/* builtin name */
	mktemp_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	mktemp_doc,		/* array of long documentation strings. */
	"mktemp [-d] [-q] [-t prefix] [-u] [-v varname] [template] ...",
	0			/* reserved for internal use */
};
