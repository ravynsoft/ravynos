/* Private interface between modules 'clean-temp-simple' and 'clean-temp'.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

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

#ifndef _CLEAN_TEMP_PRIVATE_H
#define _CLEAN_TEMP_PRIVATE_H

#include <stddef.h>
#include "gl_list.h"
#include "asyncsafe-spin.h"

/* The use of 'volatile' in the types below (and ISO C 99 section 5.1.2.3.(5))
   ensure that while constructing or modifying the data structures, the field
   values are written to memory in the order of the C statements.  So the
   signal handler can rely on these field values to be up to date.  */

/* Registry for a single temporary directory.
   'struct temp_dir' from the public header file overlaps with this.  */
struct tempdir
{
  /* The absolute pathname of the directory.  */
  char * volatile dirname;
  /* Whether errors during explicit cleanup are reported to standard error.  */
  bool cleanup_verbose;
  /* Absolute pathnames of subdirectories.  */
  gl_list_t /* <char *> */ volatile subdirs;
  /* Absolute pathnames of files.  */
  gl_list_t /* <char *> */ volatile files;
};

/* List of all temporary directories.  */
struct all_tempdirs
{
  struct tempdir * volatile * volatile tempdir_list;
  size_t volatile tempdir_count;
  size_t tempdir_allocated;
};
#define dir_cleanup_list clean_temp_dir_cleanup_list
extern struct all_tempdirs dir_cleanup_list;

/* A file descriptor to be closed.
   In multithreaded programs, it is forbidden to close the same fd twice,
   because you never know what unrelated open() calls are being executed in
   other threads. So, the 'close (fd)' must be guarded by a once-only guard.  */
struct closeable_fd
{
  /* The file descriptor to close.  */
  int volatile fd;
  /* Set to true when it has been closed.  */
  bool volatile closed;
  /* Lock that protects the fd from being closed twice.  */
  asyncsafe_spinlock_t lock;
  /* Tells whether this list element has been done and can be freed.  */
  bool volatile done;
};
#define descriptors clean_temp_descriptors
extern gl_list_t /* <closeable_fd *> */ volatile descriptors;

extern bool clean_temp_string_equals (const void *x1, const void *x2);
extern size_t clean_temp_string_hash (const void *x);

extern _GL_ASYNC_SAFE int clean_temp_asyncsafe_close (struct closeable_fd *element);
extern void clean_temp_init_asyncsafe_close (void);

extern int clean_temp_init (void);

extern int clean_temp_unlink (const char *absolute_file_name, bool cleanup_verbose);

#endif /* _CLEAN_TEMP_PRIVATE_H */
