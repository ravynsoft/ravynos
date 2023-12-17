/* logname - print login name of current user */

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

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

int
logname_builtin (list)
     WORD_LIST *list;
{
  char *np;

  if (no_options (list))
    return (EX_USAGE);

  np = getlogin ();
  if (np == 0)
    {
      builtin_error ("cannot find username: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }
  printf ("%s\n", np);
  return (EXECUTION_SUCCESS);
}

char *logname_doc[] = {
	"Display user login name.",
	"",
	"Write the current user's login name to the standard output",
	"and exit.  logname ignores the LOGNAME and USER variables.",
	"logname ignores any non-option arguments.",
	(char *)NULL
};
	
struct builtin logname_struct = {
	"logname",
	logname_builtin,
	BUILTIN_ENABLED,
	logname_doc,
	"logname",
	0
};
	
