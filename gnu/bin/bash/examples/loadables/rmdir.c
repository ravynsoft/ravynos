/* rmdir - remove directory */

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

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include "builtins.h"
#include "shell.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

int
rmdir_builtin (list)
     WORD_LIST *list;
{
  int rval;
  WORD_LIST *l;

  if (no_options (list))
    return (EX_USAGE);

  for (rval = EXECUTION_SUCCESS, l = list; l; l = l->next)
    if (rmdir (l->word->word) < 0)
      {
	builtin_error ("%s: %s", l->word->word, strerror (errno));
	rval = EXECUTION_FAILURE;
      }

  return rval;
}

char *rmdir_doc[] = {
	"Remove directory.",
	"",
	"rmdir removes the directory entry specified by each argument,",
	"provided the directory is empty.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin rmdir_struct = {
	"rmdir",		/* builtin name */
	rmdir_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	rmdir_doc,		/* array of long documentation strings. */
	"rmdir directory ...",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
