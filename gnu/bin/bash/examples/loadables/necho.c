/* necho - echo without options or argument interpretation */

/* Sample builtin to be dynamically loaded with enable -f and replace an
   existing builtin. */

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

#include <stdio.h>
#include "builtins.h"
#include "shell.h"

int
necho_builtin (list)
WORD_LIST *list;
{
	print_word_list (list, " ");
	printf("\n");
	fflush (stdout);
	return (EXECUTION_SUCCESS);
}

char *necho_doc[] = {
	"Display arguments.",
	"",
	"Print the arguments to the standard output separated",
	"by space characters and terminated with a newline.",
	(char *)NULL
};
	
struct builtin necho_struct = {
	"echo",
	necho_builtin,
	BUILTIN_ENABLED,
	necho_doc,
	"echo [args]",
	0
};
	
