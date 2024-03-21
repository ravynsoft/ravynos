/* unlink - remove a directory entry */

/* Should only be used to remove directories by a superuser prepared to let
   fsck clean up the file system. */

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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "common.h"

#ifndef errno
extern int errno;
#endif

int
unlink_builtin (list)
     WORD_LIST *list;
{
  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (unlink (list->word->word) != 0)
    {
      builtin_error ("%s: cannot unlink: %s", list->word->word, strerror (errno));
      return (EXECUTION_FAILURE);
    }

  return (EXECUTION_SUCCESS);
}

char *unlink_doc[] = {
	"Remove a directory entry.",
	"",
	"Forcibly remove a directory entry, even if it's a directory.",
	(char *)NULL
};

struct builtin unlink_struct = {
	"unlink",		/* builtin name */
	unlink_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	unlink_doc,		/* array of long documentation strings. */
	"unlink name",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
