/* mkdir - make directories */

/* See Makefile for compilation details. */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

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

#include "bashtypes.h"
#include "posixstat.h"
#include <errno.h>
#include <stdio.h>
#include "bashansi.h"
#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

#define ISOCTAL(c)	((c) >= '0' && (c) <= '7')

extern int parse_symbolic_mode ();

static int make_path ();

static int original_umask;

int
mkdir_builtin (list)
     WORD_LIST *list;
{
  int opt, pflag, mflag, omode, rval, nmode, parent_mode;
  char *mode;
  WORD_LIST *l;

  reset_internal_getopt ();
  pflag = mflag = 0;
  mode = (char *)NULL;
  while ((opt = internal_getopt(list, "m:p")) != -1)
    switch (opt)
      {
	case 'p':
	  pflag = 1;
	  break;
	case 'm':
	  mflag = 1;
	  mode = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage();
	  return (EX_USAGE);
      }
  list = loptend;

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (mode == NULL)
    omode = S_IRWXU | S_IRWXG | S_IRWXO;	/* a=rwx */
  else if (ISOCTAL (*mode))	/* octal number */
    {
      omode = read_octal (mode);
      if (omode < 0)
	{
	  builtin_error ("invalid file mode: %s", mode);
	  return (EXECUTION_FAILURE);
	}
    }
  else 				/* symbolic mode */
    {
      /* initial bits are a=rwx; the mode argument modifies them */
      omode = parse_symbolic_mode (mode, S_IRWXU | S_IRWXG | S_IRWXO);
      if (omode < 0)
	{
	  builtin_error ("invalid file mode: %s", mode);
	  return (EXECUTION_FAILURE);
	}
    }

  /* Make the new mode */
  original_umask = umask (0);
  umask (original_umask);

  nmode = (S_IRWXU | S_IRWXG | S_IRWXO) & ~original_umask;
  parent_mode = nmode | (S_IWUSR|S_IXUSR);	/* u+wx */

  /* Adjust new mode based on mode argument */
  nmode &= omode;

  for (rval = EXECUTION_SUCCESS, l = list; l; l = l->next)
    {
      if (pflag && make_path (l->word->word, mflag, nmode, parent_mode))
	{
	  rval = EXECUTION_FAILURE;
	  continue;
	}
      else if (pflag == 0 && mkdir (l->word->word, nmode) < 0)
        {
          builtin_error ("cannot create directory `%s': %s", l->word->word, strerror (errno));
          rval = EXECUTION_FAILURE;
        }
    }
  return rval;
}

/* Make all the directories leading up to PATH, then create PATH.  Note that
   this changes the process's umask; make sure that all paths leading to a
   return reset it to ORIGINAL_UMASK */
static int
make_path (path, user_mode, nmode, parent_mode)
     char *path;
     int user_mode;
     int nmode, parent_mode;
{
  int oumask;
  struct stat sb;
  char *p, *npath;

  if (stat (path, &sb) == 0)
    {
      if (S_ISDIR (sb.st_mode) == 0)
	{
	  builtin_error ("`%s': file exists but is not a directory", path);
	  return 1;
	}
	
      if (user_mode && chmod (path, nmode))
        {
          builtin_error ("%s: %s", path, strerror (errno));
          return 1;
        }

      return 0;
    }

  oumask = umask (0);
  npath = savestring (path);	/* So we can write to it. */
    
  /* Check whether or not we need to do anything with intermediate dirs. */

  /* Skip leading slashes. */
  p = npath;
  while (*p == '/')
    p++;

  while (p = strchr (p, '/'))
    {
      *p = '\0';
      if (stat (npath, &sb) != 0)
	{
	  if (mkdir (npath, 0))
	    {
	      builtin_error ("cannot create directory `%s': %s", npath, strerror (errno));
	      umask (original_umask);
	      free (npath);
	      return 1;
	    }
	  if (chmod (npath, parent_mode) != 0)
	    {
	      builtin_error ("cannot chmod directory `%s': %s", npath, strerror (errno));
	      umask (original_umask);
	      free (npath);
	      return 1;
	    }
	}
      else if (S_ISDIR (sb.st_mode) == 0)
        {
          builtin_error ("`%s': file exists but is not a directory", npath);
          umask (original_umask);
          free (npath);
          return 1;
        }

      *p++ = '/';	/* restore slash */
      while (*p == '/')
	p++;
    }

  /* Create the final directory component. */
  if (stat (npath, &sb) && mkdir (npath, nmode))
    {
      builtin_error ("cannot create directory `%s': %s", npath, strerror (errno));
      umask (original_umask);
      free (npath);
      return 1;
    }

  umask (original_umask);
  free (npath);
  return 0;
}

char *mkdir_doc[] = {
	"Create directories.",
	"",
	"Make directories.  Create the directories named as arguments, in",
	"the order specified, using mode rwxrwxrwx as modified by the current",
	"umask (see `help umask').  The -m option causes the file permission",
	"bits of the final directory to be MODE.  The MODE argument may be",
	"an octal number or a symbolic mode like that used by chmod(1).  If",
	"a symbolic mode is used, the operations are interpreted relative to",
	"an initial mode of \"a=rwx\".  The -p option causes any required",
	"intermediate directories in PATH to be created.  The directories",
	"are created with permission bits of rwxrwxrwx as modified by the current",
	"umask, plus write and search permissions for the owner.  mkdir",
	"returns 0 if the directories are created successfully, and non-zero",
	"if an error occurs.",
	(char *)NULL
};

struct builtin mkdir_struct = {
	"mkdir",
	mkdir_builtin,
	BUILTIN_ENABLED,
	mkdir_doc,
	"mkdir [-p] [-m mode] directory [directory ...]",
	0
};
