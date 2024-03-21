/* Auxiliary functions for the creation of subprocesses.  OS/2 kLIBC API.
   Copyright (C) 2001, 2003-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifndef _OS2_SPAWN_H
#define _OS2_SPAWN_H

/* Duplicates a file handle, making the copy uninheritable and ensuring the
   result is none of STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO.
   Returns -1 for a file handle that is equivalent to closed.  */
extern int dup_safer_noinherit (int fd);

/* Undoes the effect of TEMPFD = dup_safer_noinherit (ORIGFD);  */
extern void undup_safer_noinherit (int tempfd, int origfd);

/* Prepares an argument vector before calling spawn().  */
extern const char ** prepare_spawn (const char * const *argv,
                                    char **mem_to_free);

#endif /* _OS2_SPAWN_H */
