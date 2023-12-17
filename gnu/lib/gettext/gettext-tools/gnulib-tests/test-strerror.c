/* Test of strerror() function.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2007.  */

#include <config.h>

#include <string.h>

#include "signature.h"
SIGNATURE_CHECK (strerror, char *, (int));

#include <errno.h>

#include "macros.h"

int
main (void)
{
  char *str;

  errno = 0;
  str = strerror (EACCES);
  ASSERT (str);
  ASSERT (*str);
  ASSERT (errno == 0);

  errno = 0;
  str = strerror (ETIMEDOUT);
  ASSERT (str);
  ASSERT (*str);
  ASSERT (errno == 0);

  errno = 0;
  str = strerror (EOVERFLOW);
  ASSERT (str);
  ASSERT (*str);
  ASSERT (errno == 0);

  /* POSIX requires strerror (0) to succeed.  Reject use of "Unknown
     error", but allow "Success", "No error", or even Solaris' "Error
     0" which are distinct patterns from true out-of-range strings.
     http://austingroupbugs.net/view.php?id=382  */
  errno = 0;
  str = strerror (0);
  ASSERT (str);
  ASSERT (*str);
  ASSERT (errno == 0);
  ASSERT (strstr (str, "nknown") == NULL);
  ASSERT (strstr (str, "ndefined") == NULL);

  /* POSIX requires strerror to produce a non-NULL result for all
     inputs; as an extension, we also guarantee a non-empty result.
     Reporting EINVAL is optional.  */
  errno = 0;
  str = strerror (-3);
  ASSERT (str);
  ASSERT (*str);
  ASSERT (errno == 0 || errno == EINVAL);

  return 0;
}
