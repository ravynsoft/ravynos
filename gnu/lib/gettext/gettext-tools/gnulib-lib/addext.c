/* addext.c -- add an extension to a file name
   Copyright (C) 1990, 1997-1999, 2001-2003, 2005-2006, 2020 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu> and Paul Eggert */

#include <config.h>

#ifndef HAVE_DOS_FILE_NAMES
# define HAVE_DOS_FILE_NAMES 0
#endif
#ifndef HAVE_LONG_FILE_NAMES
# define HAVE_LONG_FILE_NAMES 0
#endif

#include "backupfile.h"

#include <limits.h>
#ifndef _POSIX_NAME_MAX
# define _POSIX_NAME_MAX 14
#endif

#include <sys/types.h>
#if HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#include <unistd.h>

#include "basename-lgpl.h"

/* Append to FILENAME the extension EXT, unless the result would be too long,
   in which case just append the character E.  */

void
addext (char *filename, char const *ext, char e)
{
  char *s = last_component (filename);
  size_t slen = strlen (s), extlen = strlen (ext);
  long slen_max = -1;

#if HAVE_PATHCONF && defined _PC_NAME_MAX
  if (slen + extlen <= _POSIX_NAME_MAX && ! HAVE_DOS_FILE_NAMES)
    /* The file name is so short there's no need to call pathconf.  */
    slen_max = _POSIX_NAME_MAX;
  else if (s == filename)
    slen_max = pathconf (".", _PC_NAME_MAX);
  else
    {
      char c = *s;
      *s = 0;
      slen_max = pathconf (filename, _PC_NAME_MAX);
      *s = c;
    }
#endif
  if (slen_max < 0)
    slen_max = HAVE_LONG_FILE_NAMES ? 255 : 14;

  if (HAVE_DOS_FILE_NAMES && slen_max <= 12)
    {
      /* Live within DOS's 8.3 limit.  */
      char *dot = strchr (s, '.');
      if (dot)
        {
          slen -= dot + 1 - s;
          s = dot + 1;
          slen_max = 3;
        }
      else
        slen_max = 8;
      extlen = 9; /* Don't use EXT.  */
    }

  if (slen + extlen <= slen_max)
    strcpy (s + slen, ext);
  else
    {
      if (slen_max <= slen)
        slen = slen_max - 1;
      s[slen] = e;
      s[slen + 1] = 0;
    }
}
