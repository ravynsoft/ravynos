/* savestring.c - function version of savestring for backwards compatibility */

/* Copyright (C) 1998,2003,2017 Free Software Foundation, Inc.

   This file is part of the GNU Readline Library (Readline), a library
   for reading lines of text with interactive input and history editing.      

   Readline is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Readline is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Readline.  If not, see <http://www.gnu.org/licenses/>.
*/

#define READLINE_LIBRARY

#include <config.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#include "xmalloc.h"

/* Backwards compatibility, now that savestring has been removed from
   all `public' readline header files. */
char *
savestring (const char *s)
{
  char *ret;

  ret = (char *)xmalloc (strlen (s) + 1);
  strcpy (ret, s);
  return ret;
}
