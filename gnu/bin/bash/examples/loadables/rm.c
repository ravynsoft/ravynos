/* rm - remove files and directories with -r */

/* See Makefile for compilation details. */

/*
   Copyright (C) 2016 Free Software Foundation, Inc.

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

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include "builtins.h"
#include "shell.h"
#include "common.h"
#include "bashgetopt.h"

#if !defined (errno)
extern int errno;
#endif

static int rm_file(const char *fname);

static int force, recursive;

static int
_remove_directory(const char *dirname)
{
  DIR *dir;
  struct dirent *dp;
  size_t dirlen;
  int err;

  dirlen = strlen (dirname);
  err = 0;

  if ((dir = opendir(dirname)))
    {
      while ((dp = readdir(dir)))
	{	
#ifdef __GNUC__
	  char fname[dirlen + 1 + strlen (dp->d_name) + 1];
#else
	  char *fname;
	  int fnsize;
#endif

	  QUIT;	  
          if (*dp->d_name == '.' && (dp->d_name[1] == 0 || (dp->d_name[1] == '.' && dp->d_name[2] == 0)))
            continue;

#ifdef __GNUC__
	  snprintf(fname, sizeof (fname), "%s/%s", dirname, dp->d_name);
#else
	  fnsize = dirlen + 1 + strlen (dp->d_name) + 1;
          fname = xmalloc (fnsize);
	  snprintf(fname, fnsize, "%s/%s", dirname, dp->d_name);
#endif

           if (rm_file (fname) && force == 0)
             err = 1;
#ifndef __GNUC__
           free (fname);
#endif
	   QUIT;
        }

      closedir(dir);
		
      if (err == 0 && rmdir (dirname) && force == 0)
        err = 1;
    }
  else if (force == 0)
    err = 1;

  if (err)
    builtin_error ("%s: %s", dirname, strerror (errno));

  return err;
}

static int
rm_file(const char *fname)
{
  if (unlink (fname) == 0)
    return 0;

  QUIT;
  /* If FNAME is a directory glibc returns EISDIR but correct POSIX value
     would be EPERM.  If we get that error and FNAME is a directory and -r
     was supplied, recursively remove the directory and its contents */
  if ((errno == EISDIR || errno == EPERM) && recursive && file_isdir (fname))
    return _remove_directory(fname);
  else if (force)
    return 0;

  builtin_error ("%s: %s", fname, strerror (errno));
  return 1;
}

int
rm_builtin (list)
     WORD_LIST *list;
{
  const char *name;
  WORD_LIST *l;
  int rval, opt;

  recursive = force = 0;
  rval = EXECUTION_SUCCESS;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "Rrfi")) != -1)
    {
      switch (opt)
	{
	case 'R':
	case 'r':
	  recursive = 1;
	  break;
	case 'f':
	  force = 1;
	  break;
	case 'i':
	  return (EX_DISKFALLBACK);
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (list == 0)
    {
      if (force == 0)
	{
          builtin_usage ();
          return (EXECUTION_FAILURE);
	}
      return (EXECUTION_SUCCESS);      
    }

  for (l = list; l; l = l->next)
    {
      QUIT;
      if (rm_file(l->word->word) && force == 0)
	rval = EXECUTION_FAILURE;
    }

  return rval;
}

char *rm_doc[] = {
	"Remove files.",
	"",
	"rm removes the files specified as arguments.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin rm_struct = {
	"rm",			/* builtin name */
	rm_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	rm_doc,			/* array of long documentation strings. */
	"rm [-rf] file ...",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
