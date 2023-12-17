/* Concatenate two arbitrary file names.

   Copyright (C) 1996-2007, 2009-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Jim Meyering.  */

#include <config.h>

/* Specification.  */
#include "filenamecat.h"

#include <stdlib.h>
#include <string.h>

#include "basename-lgpl.h"
#include "filename.h"

#if ! HAVE_MEMPCPY && ! defined mempcpy
# define mempcpy(D, S, N) ((void *) ((char *) memcpy (D, S, N) + (N)))
#endif

/* Concatenate two file name components, DIR and BASE, in
   newly-allocated storage and return the result.
   The resulting file name F is such that the commands "ls F" and "(cd
   DIR; ls ./BASE)" refer to the same file.  If necessary, put
   a separator between DIR and BASE in the result.  Typically this
   separator is "/", but in rare cases it might be ".".
   In any case, if BASE_IN_RESULT is non-NULL, set
   *BASE_IN_RESULT to point to the copy of BASE at the end of the
   returned concatenation.

   If malloc fails, return NULL with errno set.  */

char *
mfile_name_concat (char const *dir, char const *base, char **base_in_result)
{
  char const *dirbase = last_component (dir);
  size_t dirbaselen = base_len (dirbase);
  size_t dirlen = dirbase - dir + dirbaselen;
  size_t baselen = strlen (base);
  char sep = '\0';
  if (dirbaselen)
    {
      /* DIR is not a file system root, so separate with / if needed.  */
      if (! ISSLASH (dir[dirlen - 1]) && ! ISSLASH (*base))
        sep = '/';
    }
  else if (ISSLASH (*base))
    {
      /* DIR is a file system root and BASE begins with a slash, so
         separate with ".".  For example, if DIR is "/" and BASE is
         "/foo" then return "/./foo", as "//foo" would be wrong on
         some POSIX systems.  A fancier algorithm could omit "." in
         some cases but is not worth the trouble.  */
      sep = '.';
    }

  char *p_concat = malloc (dirlen + (sep != '\0')  + baselen + 1);
  if (p_concat == NULL)
    return NULL;

  {
    char *p;

    p = mempcpy (p_concat, dir, dirlen);
    *p = sep;
    p += sep != '\0';

    if (base_in_result)
      *base_in_result = p;

    p = mempcpy (p, base, baselen);
    *p = '\0';
  }

  return p_concat;
}
