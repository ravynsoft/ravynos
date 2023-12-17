/* Open a file, without destroying an old file with the same name.

   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible, 2020.  */

#ifndef _GL_SUPERSEDE_H
#define _GL_SUPERSEDE_H

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* When writing a file, for some usages it is important that at any moment,
   a process that opens the file will see consistent data in the file.  This
   can be important in two situations:
     * If supersede_if_exists == true, then when the file already existed,
       it is important that a process that opens the file while the new file's
       contents is being written sees consistent data - namely the old file's
       data.
     * If supersede_if_does_not_exist == true, then when the file did not exist,
       it is important that a process that opens the file while the new file's
       contents is being written sees no file (as opposed to a file with
       truncated contents).

   In both situations, the effect is implemented by creating a temporary file,
   writing into that temporary file, and renaming the temporary file when the
   temporary file's contents is complete.

   Note that opening a file with superseding may fail when it would succeed
   without superseding (for example, for a writable file in an unwritable
   directory).  And also the other way around: Opening a file with superseding
   may succeed although it would fail without superseding (for example, for
   an unwritable file in a writable directory).  */

/* This type holds everything that needs to needs to be remembered in order to
   execute the final rename action.  */
struct supersede_final_action
{
  char *final_rename_temp;
  char *final_rename_dest;
};

/* =================== open() and close() with supersede =================== */

/* The typical code idiom is like this:

     struct supersede_final_action action;
     int fd = open_supersede (filename, O_RDWR, mode,
                              supersede_if_exists, supersede_if_does_not_exist,
                              &action);
     if (fd >= 0)
       {
         ... write the file's contents ...
         if (successful)
           {
             if (close_supersede (fd, &action) < 0)
               error (...);
           }
         else
           {
             // Abort the operation.
             close (fd);
             close_supersede (-1, &action);
           }
       }
  */

/* Opens a file (typically for writing) in superseding mode, depending on
   supersede_if_exists and supersede_if_does_not_exist.
   FLAGS should not contain O_CREAT nor O_EXCL.
   MODE is used when the file does not yet exist.  The umask of the process
   is considered, like in open(), i.e. the effective mode is
   (MODE & ~ getumask ()).
   Upon success, it fills in ACTION and returns a file descriptor.
   Upon failure, it returns -1 and sets errno.  */
extern int open_supersede (const char *filename, int flags, mode_t mode,
                           bool supersede_if_exists,
                           bool supersede_if_does_not_exist,
                           struct supersede_final_action *action);

/* Closes a file and executes the final rename action.
   FD must have been returned by open_supersede(), or -1 if you want to abort
   the operation.  */
extern int close_supersede (int fd,
                            const struct supersede_final_action *action);

/* ================== fopen() and fclose() with supersede ================== */

/* The typical code idiom is like this:

     struct supersede_final_action action;
     FILE *stream =
       fopen_supersede (filename, O_RDWR, mode,
                        supersede_if_exists, supersede_if_does_not_exist,
                        &action);
     if (stream != NULL)
       {
         ... write the file's contents ...
         if (successful)
           {
             if (fclose_supersede (stream, &action) < 0)
               error (...);
           }
         else
           {
             // Abort the operation.
             fclose (stream);
             fclose_supersede (NULL, &action);
           }
       }
  */

/* Opens a file (typically for writing) in superseding mode, depending on
   supersede_if_exists and supersede_if_does_not_exist.
   Upon success, it fills in ACTION and returns a file stream.
   Upon failure, it returns NULL and sets errno.  */
extern FILE *fopen_supersede (const char *filename, const char *mode,
                              bool supersede_if_exists,
                              bool supersede_if_does_not_exist,
                              struct supersede_final_action *action);

/* Closes a file stream and executes the final rename action.
   STREAM must have been returned by fopen_supersede(), or NULL if you want to
   abort the operation.  */
extern int fclose_supersede (FILE *stream,
                             const struct supersede_final_action *action);

/* Closes a file stream, like with fwriteerror, and executes the final rename
   action.
   STREAM must have been returned by fopen_supersede(), or NULL if you want to
   abort the operation.  */
extern int fwriteerror_supersede (FILE *stream,
                                  const struct supersede_final_action *action);

#ifdef __cplusplus
}
#endif

#endif /* _GL_SUPERSEDE_H */
