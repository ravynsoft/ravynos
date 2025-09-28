/* dirname - return directory portion of pathname */

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

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include "builtins.h"
#include "shell.h"
#include "common.h"
#include "bashgetopt.h"

int
dirname_builtin (list)
     WORD_LIST *list;
{
  int slen;
  char *string;

  if (no_options (list))
    return (EX_USAGE);
  list = loptend;

  if (list == 0 || list->next)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  string = list->word->word;
  slen = strlen (string);

  /* Strip trailing slashes */
  while (slen > 0 && string[slen - 1] == '/')
    slen--;

  /* (2) If string consists entirely of slash characters, string shall be
	 set to a single slash character.  In this case, skip steps (3)
	 through (8). */
  if (slen == 0)
    {
      fputs ("/\n", stdout);
      return (EXECUTION_SUCCESS);
    }

  /* (3) If there are any trailing slash characters in string, they
	 shall be removed. */
  string[slen] = '\0';

  /* (4) If there are no slash characters remaining in string, string
	 shall be set to a single period character.  In this case, skip
	 steps (5) through (8).

     (5) If there are any trailing nonslash characters in string,
	 they shall be removed. */

  while (--slen >= 0)
    if (string[slen] == '/')
      break;

  if (slen < 0)
    {
      fputs (".\n", stdout);
      return (EXECUTION_SUCCESS);
    }

  /* (7) If there are any trailing slash characters in string, they
	 shall be removed. */
  while (--slen >= 0)
    if (string[slen] != '/')
      break;
  string[++slen] = '\0';

  /* (8) If the remaining string is empty, string shall be set to a single
	 slash character. */
  printf ("%s\n", (slen == 0) ? "/" : string);
  return (EXECUTION_SUCCESS);
}

char *dirname_doc[] = {
	"Display directory portion of pathname.",
	"",
	"The STRING is converted to the name of the directory containing",	
	"the filename corresponding to the last pathname component in STRING.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin dirname_struct = {
	"dirname",		/* builtin name */
	dirname_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	dirname_doc,		/* array of long documentation strings. */
	"dirname string",	/* usage synopsis */
	0			/* reserved for internal use */
};
