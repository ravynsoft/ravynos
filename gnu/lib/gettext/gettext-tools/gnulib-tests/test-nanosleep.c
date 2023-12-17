/* Test of nanosleep() function.
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

#include <time.h>

#include "signature.h"
SIGNATURE_CHECK (nanosleep, int, (struct timespec const *, struct timespec *));

#include <errno.h>
#include <signal.h>
#include <unistd.h>

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
  struct timespec ts;

  /* Check that negative nanosecond values cause failure.  */
  ts.tv_sec = 1;
  ts.tv_nsec = -1;
  errno = 0;
  ASSERT (nanosleep (&ts, NULL) == -1);
  ASSERT (errno == EINVAL);

  ts.tv_sec = 1000;
  ts.tv_nsec = -1;
  errno = 0;
  ASSERT (nanosleep (&ts, NULL) == -1);
  ASSERT (errno == EINVAL);

  /* Check that too large nanosecond values cause failure.  */
  ts.tv_sec = 1000;
  ts.tv_nsec = 1000000000;
  errno = 0;
  ASSERT (nanosleep (&ts, NULL) == -1);
  ASSERT (errno == EINVAL);

  /* Check successful call.  */
  ts.tv_sec = 0;
  ts.tv_nsec = 1;
  ASSERT (nanosleep (&ts, &ts) == 0);
  /* Remaining time is only defined on EINTR failure; but on success,
     it is typically either 0 or unchanged from input.  At any rate,
     it shouldn't be randomly changed to unrelated values.  */
  ASSERT (ts.tv_sec == 0);
  ASSERT (ts.tv_nsec == 0 || ts.tv_nsec == 1);
  ts.tv_nsec = 0;
  ASSERT (nanosleep (&ts, NULL) == 0);

#if HAVE_DECL_ALARM
  {
    const time_t pentecost = 50 * 24 * 60 * 60; /* 50 days.  */
    signal (SIGALRM, handle_alarm);
    alarm (1);
    ts.tv_sec = pentecost;
    ts.tv_nsec = 999999999;
    errno = 0;
    ASSERT (nanosleep (&ts, &ts) == -1);
    ASSERT (errno == EINTR);
    ASSERT (pentecost - 10 < ts.tv_sec && ts.tv_sec <= pentecost);
    ASSERT (0 <= ts.tv_nsec && ts.tv_nsec <= 999999999);
  }
#endif

  return 0;
}
