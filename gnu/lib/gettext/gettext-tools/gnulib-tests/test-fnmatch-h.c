/* Test of <fnmatch.h> substitute.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2018.  */

#include <config.h>

#include <fnmatch.h>

/* Check that the various FNM_* macros are defined.  */
int ret = FNM_NOMATCH;
int options[] = { FNM_PATHNAME, FNM_PERIOD, FNM_NOESCAPE };

int
main (void)
{
  return 0;
}
