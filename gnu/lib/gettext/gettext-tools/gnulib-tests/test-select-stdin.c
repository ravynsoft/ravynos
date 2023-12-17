/* Test of select() substitute, reading from stdin.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "macros.h"

int
main (void)
{
  printf ("Applying select() from standard input. Press Ctrl-C to abort.\n");
  for (;;)
    {
      struct timeval before;
      struct timeval after;
      unsigned long spent_usec;
      fd_set readfds;
      struct timeval timeout;
      int ret;

      gettimeofday (&before, NULL);

      FD_ZERO (&readfds);
      FD_SET (0, &readfds);
      timeout.tv_sec = 0;
      timeout.tv_usec = 500000;
      ret = select (1, &readfds, NULL, NULL, &timeout);

      gettimeofday (&after, NULL);
      spent_usec = (after.tv_sec - before.tv_sec) * 1000000
                   + after.tv_usec - before.tv_usec;

      if (ret < 0)
        {
          perror ("select failed");
          exit (1);
        }
      if ((ret == 0) != ! FD_ISSET (0, &readfds))
        {
          fprintf (stderr, "incorrect return value\n");
          exit (1);
        }
      if (ret == 0)
        {
          if (spent_usec < 250000)
            {
              fprintf (stderr, "returned too early\n");
              exit (1);
            }
          /* Timeout */
          printf (".");
          ASSERT (fflush (stdout) == 0);
        }
      else
        {
          char c;

          printf ("Input available! Trying to read 1 byte...\n");
          ASSERT (read (0, &c, 1) == 1);
        }
    }
}
