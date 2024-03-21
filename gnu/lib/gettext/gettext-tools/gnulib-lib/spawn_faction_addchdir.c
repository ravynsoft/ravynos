/* Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if REPLACE_POSIX_SPAWN
# include "spawn_int.h"
#endif

/* Add an action to FILE-ACTIONS which tells the implementation to call
   'chdir' to the given directory during the 'spawn' call.  */
int
posix_spawn_file_actions_addchdir (posix_spawn_file_actions_t *file_actions,
                                   const char *path)
#undef posix_spawn_file_actions_addchdir
{
#if !REPLACE_POSIX_SPAWN
  return posix_spawn_file_actions_addchdir_np (file_actions, path);
#else
  {
    /* Copy PATH, because the caller may free it before calling posix_spawn()
       or posix_spawnp().  */
    char *path_copy = strdup (path);
    if (path_copy == NULL)
      return ENOMEM;

    /* Allocate more memory if needed.  */
    if (file_actions->_used == file_actions->_allocated
        && __posix_spawn_file_actions_realloc (file_actions) != 0)
      {
        /* This can only mean we ran out of memory.  */
        free (path_copy);
        return ENOMEM;
      }

    {
      struct __spawn_action *rec;

      /* Add the new value.  */
      rec = &file_actions->_actions[file_actions->_used];
      rec->tag = spawn_do_chdir;
      rec->action.chdir_action.path = path_copy;

      /* Account for the new entry.  */
      ++file_actions->_used;

      return 0;
    }
  }
#endif
}
