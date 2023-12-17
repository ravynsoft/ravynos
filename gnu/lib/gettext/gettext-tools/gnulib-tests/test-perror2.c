/* Test of perror() function.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#include <errno.h>
#include <string.h>
#include <unistd.h>

/* Tell GCC not to warn about myerr being leaked.  */
#if __GNUC__ >= 13
# pragma GCC diagnostic ignored "-Wanalyzer-fd-leak"
#endif

/* This test intentionally parses stderr.  So, we arrange to have fd 10
   (outside the range of interesting fd's during the test) set up to
   duplicate the original stderr.  */
#define BACKUP_STDERR_FILENO 10
#define ASSERT_STREAM myerr
#include "macros.h"

static FILE *myerr;

#define BASE "test-perror2"

int
main (void)
{
  /* We change fd 2 later, so save it in fd 10.  */
  if (dup2 (STDERR_FILENO, BACKUP_STDERR_FILENO) != BACKUP_STDERR_FILENO
      || (myerr = fdopen (BACKUP_STDERR_FILENO, "w")) == NULL)
    return 2;

  ASSERT (freopen (BASE ".tmp", "w+", stderr) == stderr);

  /* Test that perror does not clobber strerror buffer.  */
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

    errno = EACCES;
    perror ("");
    errno = -5;
    perror ("");
    ASSERT (!ferror (stderr));
    ASSERT (STREQ (msg4, str4));

    free (str1);
    free (str2);
    free (str3);
    free (str4);
  }

  /* Test that perror uses the same message as strerror.  */
  {
    int errs[] = { EACCES, 0, -3, };
    int i;
    for (i = 0; i < SIZEOF (errs); i++)
      {
        char buf[256];
        const char *err = strerror (errs[i]);

        ASSERT (err);
        ASSERT (strlen (err) < sizeof buf);
        rewind (stderr);
        ASSERT (ftruncate (fileno (stderr), 0) == 0);
        errno = errs[i];
        perror (NULL);
        ASSERT (!ferror (stderr));
        rewind (stderr);
        ASSERT (fgets (buf, sizeof buf, stderr) == buf);
        ASSERT (strstr (buf, err));
      }
  }

  /* Test that perror reports write failure.  */
  {
    ASSERT (freopen (BASE ".tmp", "r", stderr) == stderr);
    ASSERT (setvbuf (stderr, NULL, _IONBF, BUFSIZ) == 0);
    errno = -1;
    ASSERT (!ferror (stderr));
    perror (NULL);
#if 0
    /* Commented out until cygwin behaves:
       https://sourceware.org/ml/newlib/2011/msg00228.html */
    ASSERT (errno > 0);
    /* Commented out until glibc behaves:
       https://sourceware.org/bugzilla/show_bug.cgi?id=12792 */
    ASSERT (ferror (stderr));
#endif
  }

  ASSERT (fclose (stderr) == 0);
  ASSERT (remove (BASE ".tmp") == 0);

  return 0;
}
