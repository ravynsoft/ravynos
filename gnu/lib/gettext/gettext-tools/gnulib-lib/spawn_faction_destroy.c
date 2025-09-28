/* Copyright (C) 2000, 2009-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <spawn.h>

#include <stdlib.h>

#if REPLACE_POSIX_SPAWN
# include "spawn_int.h"
#endif

/* Initialize data structure for file attribute for 'spawn' call.  */
int
posix_spawn_file_actions_destroy (posix_spawn_file_actions_t *file_actions)
#undef posix_spawn_file_actions_destroy
{
#if !REPLACE_POSIX_SPAWN
  return posix_spawn_file_actions_destroy (file_actions);
#else
  int i;

  /* Free the paths in the open actions.  */
  for (i = 0; i < file_actions->_used; ++i)
    {
      struct __spawn_action *sa = &file_actions->_actions[i];
      switch (sa->tag)
        {
        case spawn_do_open:
          free (sa->action.open_action.path);
          break;
        case spawn_do_chdir:
          free (sa->action.chdir_action.path);
          break;
        default:
          /* No cleanup required.  */
          break;
        }
    }

  /* Free the array of actions.  */
  free (file_actions->_actions);

  return 0;
#endif
}
