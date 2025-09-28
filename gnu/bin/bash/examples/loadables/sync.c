/* sync - sync the disks by forcing pending filesystem writes to complete */

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

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"

int
sync_builtin (list)
     WORD_LIST *list;
{
  sync();
  return (EXECUTION_SUCCESS);
}

char *sync_doc[] = {
	"Sync disks.",
	""
	"Force completion of pending disk writes",
	(char *)NULL
};

struct builtin sync_struct = {
	"sync",			/* builtin name */
	sync_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	sync_doc,		/* array of long documentation strings. */
	"sync",			/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
