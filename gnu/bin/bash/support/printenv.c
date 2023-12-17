/* printenv -- minimal clone of BSD printenv(1).

   usage: printenv [varname]

   Chet Ramey
   chet@po.cwru.edu
*/

/* Copyright (C) 1997-2002 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

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

#if defined (HAVE_CONFIG_H)
#  include  <config.h>
#endif

#include "bashansi.h"
#include <stdio.h>		/* puts */

extern char **environ;

int
main (argc, argv) 
     int argc;
     char **argv;
{
  register char **envp, *eval;
  int len;

  argv++;
  argc--;

  /* printenv */
  if (argc == 0)
    {
      for (envp = environ; *envp; envp++)
	puts (*envp);
      exit (0);
    }

  /* printenv varname */
  len = strlen (*argv);
  for (envp = environ; *envp; envp++)
    {
      if (**argv == **envp && strncmp (*envp, *argv, len) == 0)
	{
	  eval = *envp + len;
	  /* If the environment variable doesn't have an `=', ignore it. */
	  if (*eval == '=')
	    {
	      puts (eval + 1);
	      exit (0);
	    }
	}
    }
  exit (1);
}
  
