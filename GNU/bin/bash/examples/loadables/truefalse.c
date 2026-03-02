/* true and false builtins */

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

#include "bashtypes.h"
#include "shell.h"
#include "builtins.h"
#include "common.h"

int
true_builtin (list)
     WORD_LIST *list;
{
  return EXECUTION_SUCCESS;
}

int
false_builtin (list)
     WORD_LIST *list;
{
  return EXECUTION_FAILURE;
}

static char *true_doc[] = {
	"Exit successfully.",
	"",
	"Return a successful result.",
	(char *)NULL
};

static char *false_doc[] = {
	"Exit unsuccessfully.",
	"",
	"Return an unsuccessful result.",
	(char *)NULL
};

struct builtin true_struct = {
	"true",
	true_builtin,
	BUILTIN_ENABLED,
	true_doc,
	"true",
	0
};

struct builtin false_struct = {
	"false",
	false_builtin,
	BUILTIN_ENABLED,
	false_doc,
	"false",
	0
};
