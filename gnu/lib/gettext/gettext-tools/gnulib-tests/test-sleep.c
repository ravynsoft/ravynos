/* Test of sleep() function.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#include <unistd.h>

#include "signature.h"
SIGNATURE_CHECK (sleep, unsigned int, (unsigned int));

#include <signal.h>

#include "macros.h"

#if HAVE_DECL_ALARM
static void
handle_alarm (int sig)
{
  if (sig != SIGALRM)
    _exit (1);
}
#endif

int
main (void)
{
  ASSERT (sleep (1) <= 1);

  ASSERT (sleep (0) == 0);

#if HAVE_DECL_ALARM
  {
    const unsigned int pentecost = 50 * 24 * 60 * 60; /* 50 days.  */
    unsigned int remaining;
    signal (SIGALRM, handle_alarm);
    alarm (1);
    remaining = sleep (pentecost);
    ASSERT (pentecost - 10 < remaining && remaining <= pentecost);
  }
#endif

  return 0;
}
