/* Test of <sys/wait.h> substitute.
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

#include <sys/wait.h>

/* Check for existence of required types.  */
static pid_t a;

#include "test-sys_wait.h"

int
main (void)
{
  if (test_sys_wait_macros ())
    return 1;

#if 0
  switch (WCONTINUED)
    {
  /* Gnulib doesn't guarantee these, yet.  */
    case WCONTINUED:
    case WEXITED:
    case WNOWAIT:
    case WSTOPPED:
      break;
    }
#endif

  return a ? 1 : 0;
}
