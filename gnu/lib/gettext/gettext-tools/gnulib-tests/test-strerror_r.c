/* Test of strerror_r() function.
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

#include <config.h>

#include <string.h>

#include "signature.h"
SIGNATURE_CHECK (strerror_r, int, (int, char *, size_t));

#include <errno.h>

#include "macros.h"

int
main (void)
{
  char buf[100];
  int ret;

  /* Test results with valid errnum and enough room.  */

  errno = 0;
  buf[0] = '\0';
  ASSERT (strerror_r (EACCES, buf, sizeof buf) == 0);
  ASSERT (buf[0] != '\0');
  ASSERT (errno == 0);
  ASSERT (strlen (buf) < sizeof buf);

  errno = 0;
  buf[0] = '\0';
  ASSERT (strerror_r (ETIMEDOUT, buf, sizeof buf) == 0);
  ASSERT (buf[0] != '\0');
  ASSERT (errno == 0);
  ASSERT (strlen (buf) < sizeof buf);

  errno = 0;
  buf[0] = '\0';
  ASSERT (strerror_r (EOVERFLOW, buf, sizeof buf) == 0);
  ASSERT (buf[0] != '\0');
  ASSERT (errno == 0);
  ASSERT (strlen (buf) < sizeof buf);

  /* POSIX requires strerror (0) to succeed.  Reject use of "Unknown
     error", but allow "Success", "No error", or even Solaris' "Error
     0" which are distinct patterns from true out-of-range strings.
     http://austingroupbugs.net/view.php?id=382  */
  errno = 0;
  buf[0] = '\0';
  ret = strerror_r (0, buf, sizeof buf);
  ASSERT (ret == 0);
  ASSERT (buf[0]);
  ASSERT (errno == 0);
  ASSERT (strstr (buf, "nknown") == NULL);
  ASSERT (strstr (buf, "ndefined") == NULL);

  /* Test results with out-of-range errnum and enough room.  POSIX
     allows an empty string on success, and allows an unchanged buf on
     error, but these are not useful, so we guarantee contents.  */
  errno = 0;
  buf[0] = '^';
  ret = strerror_r (-3, buf, sizeof buf);
  ASSERT (ret == 0 || ret == EINVAL);
  ASSERT (buf[0] != '^');
  ASSERT (*buf);
  ASSERT (errno == 0);
  ASSERT (strlen (buf) < sizeof buf);

  /* Test results with a too small buffer.  POSIX requires an error;
     only ERANGE for 0 and valid errors, and a choice of ERANGE or
     EINVAL for out-of-range values.  On error, POSIX permits buf to
     be empty, unchanged, or unterminated, but these are not useful,
     so we guarantee NUL-terminated truncated contents for all but
     size 0.  http://austingroupbugs.net/view.php?id=398.  Also ensure
     that no out-of-bounds writes occur.  */
  {
    int errs[] = { EACCES, 0, -3, };
    int j;

    buf[sizeof buf - 1] = '\0';
    for (j = 0; j < SIZEOF (errs); j++)
      {
        int err = errs[j];
        char buf2[sizeof buf] = "";
        size_t len;
        size_t i;

        strerror_r (err, buf2, sizeof buf2);
        len = strlen (buf2);
        ASSERT (len < sizeof buf);

        for (i = 0; i <= len; i++)
          {
            memset (buf, '^', sizeof buf - 1);
            errno = 0;
            ret = strerror_r (err, buf, i);
            ASSERT (errno == 0);
            if (j == 2)
              ASSERT (ret == ERANGE || ret == EINVAL);
            else
              ASSERT (ret == ERANGE);
            if (i)
              {
                ASSERT (strncmp (buf, buf2, i - 1) == 0);
                ASSERT (buf[i - 1] == '\0');
              }
            ASSERT (strspn (buf + i, "^") == sizeof buf - 1 - i);
          }

        strcpy (buf, "BADFACE");
        errno = 0;
        ret = strerror_r (err, buf, len + 1);
        ASSERT (ret != ERANGE);
        ASSERT (errno == 0);
        ASSERT (strcmp (buf, buf2) == 0);
      }
  }

#if GNULIB_STRERROR
  /* Test that strerror_r does not clobber strerror buffer.  On some
     platforms, this test can only succeed if gnulib also replaces
     strerror.  */
  {
    const char *msg1;
    const char *msg2;
    const char *msg3;
    const char *msg4;
    char *str1;
    char *str2;
    char *str3;
    char *str4;

    msg1 = strerror (ENOENT);
    ASSERT (msg1);
    str1 = strdup (msg1);
    ASSERT (str1);

    msg2 = strerror (ERANGE);
    ASSERT (msg2);
    str2 = strdup (msg2);
    ASSERT (str2);

    msg3 = strerror (-4);
    ASSERT (msg3);
    str3 = strdup (msg3);
    ASSERT (str3);

    msg4 = strerror (1729576);
    ASSERT (msg4);
    str4 = strdup (msg4);
    ASSERT (str4);

    strerror_r (EACCES, buf, sizeof buf);
    strerror_r (-5, buf, sizeof buf);
    ASSERT (STREQ (msg4, str4));

    free (str1);
    free (str2);
    free (str3);
    free (str4);
  }
#endif

  return 0;
}
