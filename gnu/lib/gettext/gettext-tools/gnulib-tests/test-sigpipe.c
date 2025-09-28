/* Test of SIGPIPE handling.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#include <signal.h>

/* Check that SIGPIPE is defined.  */
int s = SIGPIPE;

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macros.h"

static void
handler (int sig)
{
  _exit (0);
}

int
main (int argc, char **argv)
{
  char mode = argv[1][0];

  switch (mode)
    {
    case 'A': signal (SIGPIPE, SIG_DFL); break;
    case 'B': signal (SIGPIPE, SIG_IGN); break;
    case 'C': signal (SIGPIPE, handler); break;
    }

  /* Produce infinite output.  Since it is piped into "head -n 1", the writes
     must ultimately fail.  */
  for (;;)
    {
      char c[2] = { 'y', '\n' };
      int ret = write (1, c, sizeof (c));
      if (ret <= 0)
        {
          switch (mode)
            {
            case 'B': /* The write() call should have failed with EPIPE.  */
              if (ret < 0 && errno == EPIPE)
                exit (0);
              FALLTHROUGH;
            case 'A': /* The process should silently die.  */
            case 'C': /* The handler should have been called.  */
              fprintf (stderr, "write() returned %d with error %d.\n", ret, errno);
              exit (1);
            }
        }
    }
}
