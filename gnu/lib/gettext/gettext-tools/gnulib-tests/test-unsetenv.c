/* Tests of unsetenv.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#include <config.h>

#include <stdlib.h>

#include "signature.h"
SIGNATURE_CHECK (unsetenv, int, (char const *));

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"

int
main (void)
{
  /* Static to pacify gcc -Wanalyzer-putenv-of-auto-var.  */
  static char entry[] = "b=2";

  /* Test removal when multiple entries present.  */
  ASSERT (putenv ((char *) "a=1") == 0);
  ASSERT (putenv (entry) == 0);
  entry[0] = 'a'; /* Unspecified what getenv("a") would be at this point.  */
  ASSERT (unsetenv ("a") == 0); /* Both entries will be removed.  */
  ASSERT (getenv ("a") == NULL);
  ASSERT (unsetenv ("a") == 0);

  /* Required to fail with EINVAL.  */
  errno = 0;
  ASSERT (unsetenv ("") == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (unsetenv ("a=b") == -1);
  ASSERT (errno == EINVAL);
#if 0
  /* glibc and gnulib's implementation guarantee this, but POSIX no
     longer requires it: http://austingroupbugs.net/view.php?id=185  */
  errno = 0;
  ASSERT (unsetenv (NULL) == -1);
  ASSERT (errno == EINVAL);
#endif

  return 0;
}
