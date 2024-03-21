/* An ftell() function that works around platform bugs.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#include <config.h>

/* Specification.  */
#include <stdio.h>

#include <errno.h>
#include <limits.h>

long
ftell (FILE *fp)
{
  /* Use the replacement ftello function with all its workarounds.  */
  off_t offset = ftello (fp);
  if (LONG_MIN <= offset && offset <= LONG_MAX)
    return /* (long) */ offset;
  else
    {
      errno = EOVERFLOW;
      return -1;
    }
}
