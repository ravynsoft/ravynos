/* strcasestr.c - Find if one string appears as a substring of another string,
		  without regard to case. */

/* Copyright (C) 2000 Free Software Foundation, Inc.

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

#include <bashansi.h>
#include <chartypes.h>

#include <stdc.h>

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive.  This is a
   case-insensitive strstr(3). */
char *
strcasestr (s1, s2)
     const char *s1;
     const char *s2;
{
  register int i, l, len, c;

  c = TOLOWER ((unsigned char)s2[0]);
  len = strlen (s1);
  l = strlen (s2);
  for (i = 0; (len - i) >= l; i++)
    if ((TOLOWER ((unsigned char)s1[i]) == c) && (strncasecmp (s1 + i, s2, l) == 0))
      return ((char *)s1 + i);
  return ((char *)0);
}
