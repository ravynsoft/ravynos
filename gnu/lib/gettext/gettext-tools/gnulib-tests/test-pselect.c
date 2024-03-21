/* Test of pselect() substitute.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#include <sys/select.h>

#include "signature.h"

SIGNATURE_CHECK (pselect, int,
                 (int, fd_set *restrict, fd_set *restrict, fd_set *restrict,
                  struct timespec const *restrict, const sigset_t *restrict));

#define TEST_PORT 12347
#include "test-select.h"

static int
my_select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout)
{
  struct timespec ts;
  struct timespec *pts = NULL;
  if (timeout)
    {
      ts.tv_sec = timeout->tv_sec;
      ts.tv_nsec = timeout->tv_usec * 1000;
      pts = &ts;
    }
  return pselect (nfds, readfds, writefds, exceptfds, pts, NULL);
}

int
main (void)
{
  return test_function (my_select);
}
