/* strnlen - return length of passed string, with length limit */

/* Copyright (C) 2004 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if !defined (HAVE_STRNLEN)

#include <sys/types.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdc.h>

/* Find the length of S, but scan at most MAXLEN characters.  If no '\0'
   terminator is found within the first MAXLEN characters, return MAXLEN. */
size_t
strnlen (s, maxlen)
     register const char *s;
     size_t maxlen;
{
  register const char *e;
  size_t n;

  for (e = s, n = 0; *e && n < maxlen; e++, n++)
    ;
  return n;
}
#endif
