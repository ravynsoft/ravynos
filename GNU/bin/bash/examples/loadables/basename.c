/* basename - return nondirectory portion of pathname */

/* See Makefile for compilation details. */

/*
   Copyright (C) 1999-2020 Free Software Foundation, Inc.

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
basename_builtin (list)
     WORD_LIST *list;
{
  int slen, sufflen, off;
  char *string, *suffix, *fn;

  if (no_options (list))
    return (EX_USAGE);
  list = loptend;
  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }    

  string = list->word->word;
  suffix = (char *)NULL;
  if (list->next)
    {
      list = list->next;
      suffix = list->word->word;
    }

  if (list->next)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  slen = strlen (string);

  /* Strip trailing slashes */
  while (slen > 0 && string[slen - 1] == '/')
    slen--;

  /* (2) If string consists entirely of slash characters, string shall be
	 set to a single slash character.  In this case, skip steps (3)
	 through (5). */
  if (slen == 0)
    {
      fputs ("/\n", stdout);
      return (EXECUTION_SUCCESS);
    }

  /* (3) If there are any trailing slash characters in string, they
	 shall be removed. */
  string[slen] = '\0';

  /* (4) If there are any slash characters remaining in string, the prefix
	 of string up to an including the last slash character in string
	 shall be removed. */
  while (--slen >= 0)
    if (string[slen] == '/')
      break;

  fn = string + slen + 1;

  /* (5) If the suffix operand is present, is not identical to the
	 characters remaining in string, and is identical to a suffix
	 of the characters remaining in string, the suffix suffix
	 shall be removed from string.  Otherwise, string shall not be
	 modified by this step. */
  if (suffix)
    {
      sufflen = strlen (suffix);
      slen = strlen (fn);
      if (sufflen < slen)
        {
          off = slen - sufflen;
          if (strcmp (fn + off, suffix) == 0)
            fn[off] = '\0';
        }
    }
  printf ("%s\n", fn);
  return (EXECUTION_SUCCESS);
}

char *basename_doc[] = {
	"Return non-directory portion of pathname.",
	"",
	"The STRING is converted to a filename corresponding to the last",
	"pathname component in STRING.  If the suffix string SUFFIX is",
	"supplied, it is removed.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin basename_struct = {
	"basename",		/* builtin name */
	basename_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	basename_doc,		/* array of long documentation strings. */
	"basename string [suffix]",	/* usage synopsis */
	0			/* reserved for internal use */
};
