/* Test posix_spawn_file_actions_addchdir() function.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

#include <spawn.h>

#include "signature.h"
SIGNATURE_CHECK (posix_spawn_file_actions_addchdir, int,
                 (posix_spawn_file_actions_t *, const char *));

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include "macros.h"

int
main (void)
{
  posix_spawn_file_actions_t actions;

  ASSERT (posix_spawn_file_actions_init (&actions) == 0);

  ASSERT (posix_spawn_file_actions_addchdir (&actions, "/") == 0);

  posix_spawn_file_actions_destroy (&actions);

  return 0;
}
