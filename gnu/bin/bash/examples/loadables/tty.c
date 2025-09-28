/* tty - return terminal name */

/* See Makefile for compilation details. */

/*
   Copyright (C) 1999-2021 Free Software Foundation, Inc.

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
#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

extern char *ttyname ();

int
tty_builtin (list)
     WORD_LIST *list;
{
  int opt, sflag;
  char *t;

  reset_internal_getopt ();
  sflag = 0;
  while ((opt = internal_getopt (list, "s")) != -1)
    {
      switch (opt)
	{
	case 's':
	  sflag = 1;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  t = ttyname (0);
  QUIT;
  if (sflag == 0)
    puts (t ? t : "not a tty");
  return (t ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

char *tty_doc[] = {
	"Display terminal name.",
	"",
	"tty writes the name of the terminal that is opened for standard",
	"input to standard output.  If the `-s' option is supplied, nothing",
	"is written; the exit status determines whether or not the standard",
	"input is connected to a tty.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin tty_struct = {
	"tty",			/* builtin name */
	tty_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	tty_doc,		/* array of long documentation strings. */
	"tty [-s]",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
