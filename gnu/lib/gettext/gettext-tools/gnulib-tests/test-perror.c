/* Test of perror() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdio.h>

#include "signature.h"
SIGNATURE_CHECK (perror, void, (char const *));

#include <errno.h>

int
main (int argc, char **argv)
{
  const char *prefix = (argc > 1 ? argv[1] : NULL);

  errno = EACCES;    perror (prefix);
  errno = ETIMEDOUT; perror (prefix);
  errno = EOVERFLOW; perror (prefix);

  return 0;
}
