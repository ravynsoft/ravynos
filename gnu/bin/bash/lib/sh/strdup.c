/* strdup - return a copy of a string in newly-allocated memory. */

/* Copyright (C) 2013 Free Software Foundation, Inc.

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

#include <config.h>

/* Get specification.  */
#include <string.h>
#include <stdlib.h>

/* Duplicate S, returning an identical malloc'd string.  */
char *
strdup (s)
     const char *s;
{
  size_t len;
  void *new;

  len = strlen (s) + 1;
  if ((new = malloc (len)) == NULL)
    return NULL;

  memcpy (new, s, len);
  return ((char *)new);
}
