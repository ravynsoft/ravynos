/* Test of <unistd.h> substitute.
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

#include <unistd.h>

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

/* Check that the various SEEK_* macros are defined.  */
int sk[] = { SEEK_CUR, SEEK_END, SEEK_SET };

/* Check that the various *_FILENO macros are defined.  */
#if ! (defined STDIN_FILENO                                     \
       && (STDIN_FILENO + STDOUT_FILENO + STDERR_FILENO == 3))
missing or broken *_FILENO macros
#endif

/* Check that the types are all defined.  */
size_t t1;
ssize_t t2;
#ifdef TODO /* Not implemented in gnulib yet */
uid_t t3;
gid_t t4;
#endif
off_t t5;
pid_t t6;
#ifdef TODO
useconds_t t7;
intptr_t t8;
#endif

int
main (void)
{
  return 0;
}
