/*
 * whoami - print out username of current user
 */

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
#include <stdio.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

int
whoami_builtin (list)
     WORD_LIST *list;
{
  int opt;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "")) != -1)
    {
      switch (opt)
	{
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;
  if (list)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (current_user.user_name == 0)
    get_current_user_info ();
  printf ("%s\n", current_user.user_name);
  return (EXECUTION_SUCCESS);
}

char *whoami_doc[] = {
	"Print user name",
	"",
	"Display name of current user.",
	(char *)NULL
};

struct builtin whoami_struct = {
	"whoami",
	whoami_builtin,
	BUILTIN_ENABLED,
	whoami_doc,
	"whoami",
	0
};
