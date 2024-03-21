/* Test opening a stream with a file descriptor.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (fdopen, FILE *, (int, const char *));

#include <errno.h>
#include <unistd.h>

#include "macros.h"

int
main (void)
{
  /* Test behavior on failure.  POSIX makes it hard to check for
     failure, since the behavior is not well-defined on invalid file
     descriptors, so try fdopen 1000 times and if that's not enough to
     fail due to EMFILE, so be it.  */
  #if defined __ANDROID__ /* fdsan */
    #define COUNT 1
  #else
    #define COUNT 1000
  #endif

  int i;
  for (i = 0; i < COUNT; i++)
    {
      errno = 0;
      if (! fdopen (STDOUT_FILENO, "w"))
        {
          ASSERT (errno != 0);
          break;
        }
    }

  return 0;
}
