/* Test of getcwd() function.
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

#include <config.h>

#include <unistd.h>

#include "signature.h"
SIGNATURE_CHECK (getcwd, char *, (char *, size_t));

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

int
main (int argc, char **argv)
{
  char *pwd1;
  char *pwd2;
  /* If the user provides an argument, attempt to chdir there first.  */
  if (1 < argc)
    {
      if (chdir (argv[1]) == 0)
        printf ("changed to directory %s\n", argv[1]);
    }

  pwd1 = getcwd (NULL, 0);
  ASSERT (pwd1 && *pwd1);
  if (1 < argc)
    printf ("cwd=%s\n", pwd1);

  /* Make sure the result is usable.  */
  ASSERT (chdir (pwd1) == 0);
  ASSERT (chdir (".//./.") == 0);

  /* Make sure that result is normalized.  */
  pwd2 = getcwd (NULL, 0);
  ASSERT (pwd2);
  ASSERT (strcmp (pwd1, pwd2) == 0);
  free (pwd2);
  {
    size_t len = strlen (pwd1);
    ssize_t i = len - 10;
    if (i < 1)
      i = 1;
    pwd2 = getcwd (NULL, len + 1);
    ASSERT (pwd2);
    free (pwd2);
    pwd2 = malloc (len + 2);
    for ( ; i <= len; i++)
      {
        char *tmp;
        errno = 0;
        ASSERT (getcwd (pwd2, i) == NULL);
        ASSERT (errno == ERANGE);
        /* Allow either glibc or BSD behavior, since POSIX allows both.  */
        errno = 0;
        tmp = getcwd (NULL, i);
        if (tmp)
          {
            ASSERT (strcmp (pwd1, tmp) == 0);
            free (tmp);
          }
        else
          {
            ASSERT (errno == ERANGE);
          }
      }
    ASSERT (getcwd (pwd2, len + 1) == pwd2);
    pwd2[len] = '/';
    pwd2[len + 1] = '\0';
  }
  ASSERT (strstr (pwd2, "/./") == NULL);
  ASSERT (strstr (pwd2, "/../") == NULL);
  ASSERT (strstr (pwd2 + 1 + (pwd2[1] == '/'), "//") == NULL);

  /* Validate a POSIX requirement on size.  */
  errno = 0;
  ASSERT (getcwd (pwd2, 0) == NULL);
  ASSERT (errno == EINVAL);

  free (pwd1);
  free (pwd2);

  return 0;
}
