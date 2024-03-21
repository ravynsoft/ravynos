/* Test of <sys/time.h> substitute.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <sys/time.h>

/* Check that the 'struct timeval' type is defined.  */
struct timeval a;

/* Check that a.tv_sec is wide enough to hold a time_t, ignoring
   signedness issues.  */
typedef int verify_tv_sec_type[sizeof (time_t) <= sizeof (a.tv_sec) ? 1 : -1];

int
main (void)
{
  return 0;
}
