/* Test of <stdlib.h> substitute.
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

#include <stdlib.h>

/* Check that EXIT_SUCCESS is 0, per POSIX.  */
static int exitcode = EXIT_SUCCESS;
#if EXIT_SUCCESS
"oops"
#endif

/* Check for GNU value (not guaranteed by POSIX, but is guaranteed by
   gnulib).  */
#if EXIT_FAILURE != 1
"oops"
#endif

/* Check that NULL can be passed through varargs as a pointer type,
   per POSIX 2008.  */
static_assert (sizeof NULL == sizeof (void *));

#if GNULIB_TEST_SYSTEM_POSIX
# include "test-sys_wait.h"
#else
# define test_sys_wait_macros() 0
#endif

int
main (void)
{
  /* POSIX:2018 says:
     "In the POSIX locale the value of MB_CUR_MAX shall be 1."  */
  /* On Android ≥ 5.0, the default locale is the "C.UTF-8" locale, not the
     "C" locale.  Furthermore, when you attempt to set the "C" or "POSIX"
     locale via setlocale(), what you get is a "C" locale with UTF-8 encoding,
     that is, effectively the "C.UTF-8" locale.  */
#ifndef __ANDROID__
  if (MB_CUR_MAX != 1)
    return 1;
#endif

  if (test_sys_wait_macros ())
    return 2;

  return exitcode;
}
