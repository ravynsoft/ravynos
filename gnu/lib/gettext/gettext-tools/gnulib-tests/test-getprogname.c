/* Test the gnulib getprogname module.
   Copyright (C) 2016-2023 Free Software Foundation, Inc.

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

#include <stdlib.h>

#include <string.h>
#include <assert.h>

#ifdef __hpux
# define STREQ(a, b) (strncmp (a, b, 14) == 0)
#else
# define STREQ(a, b) (strcmp (a, b) == 0)
#endif

int
main (void)
{
  char const *p = getprogname ();

  /* libtool creates a temporary executable whose name is sometimes prefixed
     with "lt-" (depends on the platform).  But the name of the temporary
     executable is a detail that should not be visible to the end user and to
     the test suite.  Remove this "lt-" prefix here.  */
  if (strncmp (p, "lt-", 3) == 0)
    p += 3;

  /* Note: You can make this test fail
     a) by running it on a case-insensitive file system (such as on Windows,
        Cygwin, or on Mac OS X with a case-insensitive HFS+ file system),
        with an invocation that contains upper case characters, e.g.
        test-GETPROGNAME,
     b) by hardlinking or symlinking it to a different name (e.g. test-foo)
        and invoking it through that name.
     That's not the intended use. The Makefile always invokes it as
     'test-getprogname${EXEEXT}'. */
#if defined __CYGWIN__
  /* The Cygwin getprogname() function strips the ".exe" suffix. */
  assert (STREQ (p, "test-getprogname"));
#else
  assert (STREQ (p, "test-getprogname" EXEEXT));
#endif

  return 0;
}
