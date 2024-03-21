/*
 * Copyright (C) 2006-2007, 2010-2023 Free Software Foundation, Inc.
 * Written by Simon Josefsson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include "read-file.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "macros.h"

#define FILE1 "/etc/resolv.conf"
#define FILE2 "/dev/null"

static int
test_read_file (int flags)
{
  struct stat statbuf;
  int err = 0;

  /* We can perform the test only if the file exists and is readable.
     Test whether it exists, then assume it is world-readable.  */
  if (stat (FILE1, &statbuf) >= 0)
    {
      size_t len;
      char *out = read_file (FILE1, flags, &len);

      if (!out)
        {
          perror ("Could not read file");
          err = 1;
        }
      else
        {
          if (out[len] != '\0')
            {
              perror ("BAD: out[len] not zero");
              err = 1;
            }

          if (S_ISREG (statbuf.st_mode))
            {
              /* FILE1 is a regular file or a symlink to a regular file.  */
              if (len != statbuf.st_size)
                {
                  fprintf (stderr, "Read %lu from %s...\n",
                           (unsigned long) len, FILE1);
                  err = 1;
                }
            }
          else
            {
              /* Assume FILE1 is not empty.  */
              if (len == 0)
                {
                  fprintf (stderr, "Read nothing from %s\n", FILE1);
                  err = 1;
                }
            }
          free (out);
        }
    }

  /* We can perform the test only if the file exists and is readable.
     Test whether it exists, then assume it is world-readable.  */
  if (stat (FILE2, &statbuf) >= 0)
    {
      size_t len;
      char *out = read_file (FILE2, flags, &len);

      if (!out)
        {
          perror ("Could not read file");
          err = 1;
        }
      else
        {
          if (out[len] != '\0')
            {
              perror ("BAD: out[len] not zero");
              err = 1;
            }

          /* /dev/null should always be empty.  Ignore statbuf.st_size, since it
             is not a regular file.  */
          if (len != 0)
            {
              fprintf (stderr, "Read %lu from %s...\n",
                       (unsigned long) len, FILE2);
              err = 1;
            }
          free (out);
        }
    }

  return err;
}

int
main (void)
{
  ASSERT (!test_read_file (0));
  ASSERT (!test_read_file (RF_BINARY));
  ASSERT (!test_read_file (RF_SENSITIVE));
  ASSERT (!test_read_file (RF_BINARY | RF_SENSITIVE));

  return 0;
}
