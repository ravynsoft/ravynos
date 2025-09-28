/* Test of <time.h> substitute.
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

#include <time.h>

/* Check that the types are all defined.  */
struct timespec t1;
#if 0
/* POSIX:2008 does not require pid_t in <time.h> unconditionally, and indeed
   it's missing on Mac OS X 10.5, FreeBSD 6.4, OpenBSD 4.9, mingw.  */
pid_t t2;
#endif

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

/* Check that TIME_UTC is defined and a positive integer.  */
int t3 = TIME_UTC;
static_assert (TIME_UTC > 0);

int
main (void)
{
  return 0;
}
