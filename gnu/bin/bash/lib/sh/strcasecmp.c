/* strcasecmp.c - functions for case-insensitive string comparison. */

/* Copyright (C) 1995 Free Software Foundation, Inc.

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

#if !defined (HAVE_STRCASECMP)

#include <stdc.h>
#include <bashansi.h>
#include <chartypes.h>

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
strncasecmp (string1, string2, count)
     const char *string1;
     const char *string2;
     size_t count;
{
  register const char *s1;
  register const char *s2;
  register int r;

  if (count <= 0 || (string1 == string2))
    return 0;

  s1 = string1;
  s2 = string2;
  do
    {
      if ((r = TOLOWER ((unsigned char) *s1) - TOLOWER ((unsigned char) *s2)) != 0)
	return r;
      if (*s1++ == '\0')
	break;
      s2++;
    }
  while (--count != 0);

  return (0);
}

/* strcmp (), but caseless. */
int
strcasecmp (string1, string2)
     const char *string1;
     const char *string2;
{
  register const char *s1;
  register const char *s2;
  register int r;

  s1 = string1;
  s2 = string2;

  if (s1 == s2)
    return (0);

  while ((r = TOLOWER ((unsigned char)*s1) - TOLOWER ((unsigned char)*s2)) == 0)
    {
      if (*s1++ == '\0')
	return 0;
      s2++;
    }

  return (r);
}
#endif /* !HAVE_STRCASECMP */
