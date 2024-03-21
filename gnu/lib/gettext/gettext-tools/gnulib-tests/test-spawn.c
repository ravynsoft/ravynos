/* Test of <spawn.h> substitute.
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

#include <spawn.h>

/* Check for existence of required types.  */
struct check
{
  posix_spawnattr_t a;
  posix_spawn_file_actions_t b;
  mode_t c;
  pid_t d;
  sigset_t e;
} s;

/* struct sched_param is allowed to be an incomplete type without
   <sched.h>, but must have a forward declaration to avoid a
   compilation error in the following usage.  */
extern void f (struct sched_param *g);
#include <sched.h>
extern void f (struct sched_param *g);

int
main (void)
{
  switch (POSIX_SPAWN_RESETIDS)
    {
    case POSIX_SPAWN_RESETIDS:
    case POSIX_SPAWN_SETPGROUP:
    case POSIX_SPAWN_SETSIGDEF:
    case POSIX_SPAWN_SETSIGMASK:
    case (POSIX_SPAWN_SETSCHEDPARAM != 0 ? POSIX_SPAWN_SETSCHEDPARAM : -1):
    case (POSIX_SPAWN_SETSCHEDULER != 0 ? POSIX_SPAWN_SETSCHEDULER : -2):
      ;
    }
  return s.c + s.d;
}
