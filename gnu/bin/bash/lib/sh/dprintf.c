/* dprintf -- printf to a file descriptor */

/* Copyright (C) 2008-2010 Free Software Foundation, Inc.

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
#  include <config.h>
#endif

#include <stdc.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (PREFER_STDARG)
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#include <stdio.h>

int
#if defined (PREFER_STDARG)
dprintf(int fd, const char *format, ...)
#else
dprintf(fd, format, va_alist)
     int fd;
     const char *format;
     va_dcl
#endif
{
  FILE *fp;
  int fd2, rc, r2;
  va_list args;

  if ((fd2 = dup(fd)) < 0)
    return -1;
  fp = fdopen (fd2, "w");
  if (fp == 0)
    {
      close (fd2);
      return -1;
    }

  SH_VA_START (args, format);
  rc = vfprintf (fp, format, args);
  fflush (fp);
  va_end (args);

  r2 = fclose (fp);	/* check here */

  return rc;
}           
