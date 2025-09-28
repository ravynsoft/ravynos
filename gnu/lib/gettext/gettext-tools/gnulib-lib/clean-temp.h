/* Temporary directories and temporary files with automatic cleanup.
   Copyright (C) 2006, 2011-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _CLEAN_TEMP_H
#define _CLEAN_TEMP_H

/* This file uses _GL_ATTRIBUTE_DEALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Temporary directories and temporary files should be automatically removed
   when the program exits either normally or through a fatal signal.  We can't
   rely on the "unlink before close" idiom, because it works only on Unix and
   also - if no signal blocking is used - leaves a time window where a fatal
   signal would not clean up the temporary file.

   Also, open file descriptors need to be closed before the temporary files
   and the temporary directories can be removed, because only on Unix
   (excluding Cygwin) can one remove directories containing open files.

   There are two modules:
     - 'clean-temp' provides support for temporary directories and temporary
       files inside these temporary directories,
     - 'clean-temp-simple' provides support for temporary files without
       temporary directories.
   The temporary directories and files are automatically cleaned up (at the
   latest) when the program exits or dies from a fatal signal such as SIGINT,
   SIGTERM, SIGHUP, but not if it dies from a fatal signal such as SIGQUIT,
   SIGKILL, or SIGABRT, SIGSEGV, SIGBUS, SIGILL, SIGFPE.

   For the cleanup in the normal case, programs that use this module need to
   call 'cleanup_temp_dir' for each successful return of 'create_temp_dir'.
   The cleanup in the case of a fatal signal such as SIGINT, SIGTERM, SIGHUP,
   is done entirely automatically by the functions of this module.

   Limitations: Files or directories can still be left over if
     - the program is dies from a fatal signal such as SIGQUIT, SIGKILL, or
       SIGABRT, SIGSEGV, SIGBUS, SIGILL, SIGFPE, or
     - in a multithreaded program, the fatal signal handler is already running
       while another thread of the program creates a new temporary directory
       or temporary file, or
     - on native Windows, some temporary files are used by a subprocess while
       the fatal signal interrupts the program.
 */


/* ============= Temporary files without temporary directories ============= */

#include "clean-temp-simple.h"

/* ========= Temporary directories and temporary files inside them ========= */

struct temp_dir
{
  /* The absolute pathname of the directory.  */
  const char * const dir_name;
  /* Whether errors during explicit cleanup are reported to standard error.  */
  bool cleanup_verbose;
  /* More fields are present here, but not public.  */
};

/* Remove all registered files and subdirectories inside DIR and DIR itself.
   DIR cannot be used any more after this call.
   Return 0 upon success, or -1 if there was some problem.  */
extern int cleanup_temp_dir (struct temp_dir *dir);

/* Create a temporary directory.
   PREFIX is used as a prefix for the name of the temporary directory. It
   should be short and still give an indication about the program.
   PARENTDIR can be used to specify the parent directory; if NULL, a default
   parent directory is used (either $TMPDIR or /tmp or similar).
   CLEANUP_VERBOSE determines whether errors during explicit cleanup are
   reported to standard error.
   Return a fresh 'struct temp_dir' on success.  Upon error, an error message
   is shown and NULL is returned.  */
extern struct temp_dir * create_temp_dir (const char *prefix,
                                          const char *parentdir,
                                          bool cleanup_verbose)
  _GL_ATTRIBUTE_DEALLOC (cleanup_temp_dir, 1);

/* Register the given ABSOLUTE_FILE_NAME as being a file inside DIR, that
   needs to be removed before DIR can be removed.
   Should be called before the file ABSOLUTE_FILE_NAME is created.  */
extern void register_temp_file (struct temp_dir *dir,
                                const char *absolute_file_name);

/* Unregister the given ABSOLUTE_FILE_NAME as being a file inside DIR, that
   needs to be removed before DIR can be removed.
   Should be called when the file ABSOLUTE_FILE_NAME could not be created.  */
extern void unregister_temp_file (struct temp_dir *dir,
                                  const char *absolute_file_name);

/* Register the given ABSOLUTE_DIR_NAME as being a subdirectory inside DIR,
   that needs to be removed before DIR can be removed.
   Should be called before the subdirectory ABSOLUTE_DIR_NAME is created.  */
extern void register_temp_subdir (struct temp_dir *dir,
                                  const char *absolute_dir_name);

/* Unregister the given ABSOLUTE_DIR_NAME as being a subdirectory inside DIR,
   that needs to be removed before DIR can be removed.
   Should be called when the subdirectory ABSOLUTE_DIR_NAME could not be
   created.  */
extern void unregister_temp_subdir (struct temp_dir *dir,
                                    const char *absolute_dir_name);

/* Remove the given ABSOLUTE_FILE_NAME and unregister it.
   Return 0 upon success, or -1 if there was some problem.  */
extern int cleanup_temp_file (struct temp_dir *dir,
                              const char *absolute_file_name);

/* Remove the given ABSOLUTE_DIR_NAME and unregister it.
   Return 0 upon success, or -1 if there was some problem.  */
extern int cleanup_temp_subdir (struct temp_dir *dir,
                                const char *absolute_dir_name);

/* Remove all registered files and subdirectories inside DIR.
   Return 0 upon success, or -1 if there was some problem.  */
extern int cleanup_temp_dir_contents (struct temp_dir *dir);

/* ================== Opening and closing temporary files ================== */

/* Open a temporary file in a temporary directory.
   FILE_NAME must already have been passed to register_temp_file.
   Registers the resulting file descriptor to be closed.
   DELETE_ON_CLOSE indicates whether the file can be deleted when the resulting
   file descriptor or stream is closed.  */
extern int open_temp (const char *file_name, int flags, mode_t mode,
                      bool delete_on_close);
extern FILE * fopen_temp (const char *file_name, const char *mode,
                          bool delete_on_close);

/* Open a temporary file, generating its name based on FILE_NAME_TMPL.
   FILE_NAME_TMPL must match the rules for mk[s]temp (i.e. end in "XXXXXX",
   possibly with a suffix).  The name constructed does not exist at the time
   of the call.  FILE_NAME_TMPL is overwritten with the result.
   A safe choice for MODE is S_IRUSR | S_IWUSR, a.k.a. 0600.
   Registers the file for deletion.
   Opens the file, with the given FLAGS and mode MODE.
   Registers the resulting file descriptor to be closed.  */
extern int gen_register_open_temp (char *file_name_tmpl, int suffixlen,
                                   int flags, mode_t mode);

/* Close a temporary file.
   FD must have been returned by open_temp or gen_register_open_temp.
   Unregisters the previously registered file descriptor.  */
extern int close_temp (int fd);

/* Close a temporary file.
   FP must have been returned by fopen_temp, or by fdopen on a file descriptor
   returned by open_temp or gen_register_open_temp.
   Unregisters the previously registered file descriptor.  */
extern int fclose_temp (FILE *fp);

/* Like fwriteerror.
   FP must have been returned by fopen_temp, or by fdopen on a file descriptor
   returned by open_temp or gen_register_open_temp.
   Unregisters the previously registered file descriptor.  */
extern int fwriteerror_temp (FILE *fp);

/* Like close_stream.
   FP must have been returned by fopen_temp, or by fdopen on a file descriptor
   returned by open_temp or gen_register_open_temp.
   Unregisters the previously registered file descriptor.  */
extern int close_stream_temp (FILE *fp);


#ifdef __cplusplus
}
#endif

#endif /* _CLEAN_TEMP_H */
